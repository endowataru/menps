
#pragma once

#include "bounded_queue_counter.hpp"
#include <menps/mefdn/algorithm.hpp>

namespace menps {
namespace mefdn {

namespace detail {

template <typename Element>
struct spmc_bounded_queue_entry
{
    spmc_bounded_queue_entry()
        : elem{}
        , used{} { }
    
    Element                 elem;
    mefdn::atomic<bool>    used;
};

template <typename Traits>
struct spmc_bounded_queue_traits
    : Traits
{
    typedef typename Traits::element_type           element_type;
    
    typedef spmc_bounded_queue_entry<element_type>  entry_type;
};

template <typename Traits>
class spmc_bounded_queue_base
    : public bounded_queue_counter<spmc_bounded_queue_traits<Traits>>
{
    typedef spmc_bounded_queue_traits<Traits>   traits_type;
    typedef bounded_queue_counter<traits_type>  base;
    
protected:
    typedef typename base::derived_type     derived_type;
    typedef typename base::index_type       index_type;
    typedef typename base::element_type     element_type;
    typedef typename base::entry_type       entry_type;
    
    typedef typename base::enqueue_transaction_base enqueue_transaction_base;
    typedef typename base::enqueue_transaction_data enqueue_transaction_data;
    typedef typename base::dequeue_transaction_base dequeue_transaction_base;
    typedef typename base::dequeue_transaction_data dequeue_transaction_data;
    
protected:
    spmc_bounded_queue_base() noexcept = default;
    
public:
    class enqueue_transaction
        : public enqueue_transaction_base
    {
    public:
        enqueue_transaction() noexcept = default;
        
        enqueue_transaction(
            derived_type&                   self
        ,   const enqueue_transaction_data& data
        ) noexcept
            : enqueue_transaction_base(self, data) { }
        
        enqueue_transaction(const enqueue_transaction&) = delete;
        enqueue_transaction& operator = (const enqueue_transaction&) = delete;
        
        enqueue_transaction(enqueue_transaction&&) noexcept = default;
        enqueue_transaction& operator = (enqueue_transaction&&) noexcept = default;
        
        void commit(const index_type num)
        {
            MEFDN_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MEFDN_ASSERT(num <= static_cast<index_type>(last - itr));
            
            for (index_type i = 0; i < num; ++i, ++itr)
            {
                // The memory barrier is done when head_ is replaced.
                // Hence we use relaxed memory ordering here.
                itr->used.store(true, mefdn::memory_order_relaxed);
            }
            
            auto& self = *this->self_;
            
            const auto new_tail = this->data_.from + num;
            
            // Allow producers to use the elements.
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.tail_.store(new_tail, mefdn::memory_order_release);
            
            MEFDN_LOG_VERBOSE(
                "msg:Finished enqueuing.\t"
            );
            
            enqueue_transaction_base::commit();
        }
    };
    
    enqueue_transaction try_enqueue(const index_type num)
    {
        const auto cap = derived().capacity();
        
        const auto head = this->head_.load(mefdn::memory_order_acquire);
        const auto tail = this->tail_.load(mefdn::memory_order_relaxed);
        
        const auto new_tail = tail + num;
        
        const auto new_size = new_tail - head;
        
        if (MEFDN_UNLIKELY(new_size >= cap)) {
            return {};
        }
        
        auto itr = this->make_entry_iterator_at(tail);
        const auto last = this->make_entry_iterator_at(new_tail);
        
        for ( ; itr != last; ++itr)
        {
            const bool is_used = itr->used.load(mefdn::memory_order_acquire);
            
            if (MEFDN_UNLIKELY(is_used)) {
                return {};
            }
        }
        
        // All of the flags "is_used"
        // in the range [tail, new_tail) are "false".
        
        return enqueue_transaction(derived(), { tail, new_tail });
    }
    
    class dequeue_transaction
        : public dequeue_transaction_base
    {
    public:
        dequeue_transaction() noexcept = default;
        
        dequeue_transaction(
            derived_type&                   self
        ,   const dequeue_transaction_data& data
        ) noexcept
            : dequeue_transaction_base(self, data)
            { }
        
        dequeue_transaction(const dequeue_transaction&) = delete;
        dequeue_transaction& operator = (const dequeue_transaction&) = delete;
        
        dequeue_transaction(dequeue_transaction&&) noexcept = default;
        dequeue_transaction& operator = (dequeue_transaction&&) noexcept = default;
        
        void commit(const index_type num)
        {
            MEFDN_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MEFDN_ASSERT(num == static_cast<index_type>(last - itr));
            
            for ( ; itr != last; ++itr)
            {
                // TODO: reduce memory barriers
                itr->used.store(false, mefdn::memory_order_release);
            }
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        index_type head = this->head_.load(mefdn::memory_order_relaxed);
        
        while (true)
        {
            const auto tail =
                this->tail_.load(mefdn::memory_order_acquire);
            
            const auto num_enqueued = tail - head;
            
            if (num_enqueued <= 0) {
                return {};
            }
            
            const auto num_dequeued =
                mefdn::min(num, num_enqueued);
            
            const auto new_head = head + num_dequeued;
            
            // Try to store the new head.
            
            const bool success =
                this->head_.compare_exchange_weak(
                    head
                ,   new_head
                ,   mefdn::memory_order_acquire
                ,   mefdn::memory_order_relaxed
                );
            
            // Now head_ is written to head by compare_exchange_weak.
            
            if (MEFDN_LIKELY(success))
            {
                return dequeue_transaction(derived(), { head, new_head });
            }
        }
    }
    
private:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mefdn
} // namespace menps

