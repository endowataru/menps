
#pragma once

#include "bounded_queue_counter.hpp"
#include <mgbase/algorithm.hpp>

namespace mgbase {

namespace detail {

template <typename Element>
struct spmc_bounded_queue_entry
{
    spmc_bounded_queue_entry()
        : elem{}
        , used{} { }
    
    Element                 elem;
    mgbase::atomic<bool>    used;
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
    spmc_bounded_queue_base() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    class enqueue_transaction
        : public enqueue_transaction_base
    {
    public:
        enqueue_transaction() MGBASE_DEFAULT_NOEXCEPT = default;
        
        enqueue_transaction(
            derived_type&                   self
        ,   const enqueue_transaction_data& data
        ) MGBASE_NOEXCEPT
            : enqueue_transaction_base(self, data) { }
        
        enqueue_transaction(const enqueue_transaction&) = delete;
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        enqueue_transaction(enqueue_transaction&&) MGBASE_DEFAULT_NOEXCEPT = default;
        #else
        enqueue_transaction(enqueue_transaction&& other) MGBASE_NOEXCEPT
            : enqueue_transaction_base(mgbase::move(other)) { }
        #endif
        
        void commit(const index_type num)
        {
            MGBASE_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MGBASE_ASSERT(num <= static_cast<index_type>(last - itr));
            
            for (index_type i = 0; i < num; ++i, ++itr)
            {
                // The memory barrier is done when head_ is replaced.
                // Hence we use relaxed memory ordering here.
                itr->used.store(true, mgbase::memory_order_relaxed);
            }
            
            auto& self = *this->self_;
            
            const auto new_tail = this->data_.from + num;
            
            // Allow producers to use the elements.
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.tail_.store(new_tail, mgbase::memory_order_release);
            
            MGBASE_LOG_VERBOSE(
                "msg:Finished dequeuing.\t"
            );
            
            enqueue_transaction_base::commit();
        }
    };
    
    enqueue_transaction try_enqueue(const index_type num)
    {
        const auto cap = derived().capacity();
        
        const auto head = this->head_.load(mgbase::memory_order_acquire);
        const auto tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        const auto new_tail = tail + num;
        
        const auto new_size = new_tail - head;
        
        if (MGBASE_UNLIKELY(new_size >= cap)) {
            return {};
        }
        
        auto itr = this->make_entry_iterator_at(tail);
        const auto last = this->make_entry_iterator_at(new_tail);
        
        for ( ; itr != last; ++itr)
        {
            const bool is_used = itr->used.load(mgbase::memory_order_acquire);
            
            if (MGBASE_UNLIKELY(is_used)) {
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
        dequeue_transaction() MGBASE_DEFAULT_NOEXCEPT = default;
        
        dequeue_transaction(
            derived_type&                   self
        ,   const dequeue_transaction_data& data
        ) MGBASE_NOEXCEPT
            : dequeue_transaction_base(self, data)
            { }
        
        dequeue_transaction(const dequeue_transaction&) = delete;
        
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        dequeue_transaction(dequeue_transaction&&) MGBASE_DEFAULT_NOEXCEPT = default;
        #else
        dequeue_transaction(dequeue_transaction&& other) MGBASE_NOEXCEPT
            : dequeue_transaction_base(mgbase::move(other)) { }
        #endif
        
        void commit(const index_type num)
        {
            MGBASE_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MGBASE_ASSERT(num == static_cast<index_type>(last - itr));
            
            for ( ; itr != last; ++itr)
            {
                // TODO: reduce memory barriers
                itr->used.store(false, mgbase::memory_order_release);
            }
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        index_type head = this->head_.load(mgbase::memory_order_relaxed);
        
        while (true)
        {
            const auto tail =
                this->tail_.load(mgbase::memory_order_acquire);
            
            const auto num_enqueued = tail - head;
            
            if (num_enqueued <= 0) {
                return {};
            }
            
            const auto num_dequeued =
                mgbase::min(num, num_enqueued);
            
            const auto new_head = head + num_dequeued;
            
            // Try to store the new head.
            
            const bool success =
                this->head_.compare_exchange_weak(
                    head
                ,   new_head
                ,   mgbase::memory_order_acquire
                ,   mgbase::memory_order_relaxed
                );
            
            // Now head_ is written to head by compare_exchange_weak.
            
            if (MGBASE_LIKELY(success))
            {
                return dequeue_transaction(derived(), { head, new_head });
            }
        }
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    entry_type* entries_;
};

} // namespace detail

} // namespace mgbase

