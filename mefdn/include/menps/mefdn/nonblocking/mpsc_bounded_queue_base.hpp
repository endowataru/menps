
#pragma once

#include "bounded_queue_counter.hpp"

namespace menps {
namespace mefdn {

namespace detail {

template <typename Element>
struct mpsc_bounded_queue_entry
{
    Element                 elem;
    mefdn::atomic<bool>    state;
};

template <typename Traits>
struct mpsc_bounded_queue_traits
    : Traits
{
    typedef typename Traits::element_type           element_type;
    
    typedef mpsc_bounded_queue_entry<element_type>  entry_type;
};

template <typename Traits>
class mpsc_bounded_queue_base
    : public bounded_queue_counter<mpsc_bounded_queue_traits<Traits>>
{
    typedef mpsc_bounded_queue_traits<Traits>   traits_type;
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
    mpsc_bounded_queue_base() noexcept = default;
    
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
            : enqueue_transaction_base(self, data)
            { }
        
        enqueue_transaction(const enqueue_transaction&) = delete;
        enqueue_transaction& operator = (const enqueue_transaction&) = delete;
        
        enqueue_transaction(enqueue_transaction&&) noexcept = default;
        enqueue_transaction& operator = (enqueue_transaction&&) noexcept = default;
        
        void commit(MEFDN_MAYBE_UNUSED const index_type num)
        {
            MEFDN_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MEFDN_ASSERT(num == static_cast<index_type>(last - itr));
            
            for ( ; itr != last; ++itr)
            {
                // TODO: reduce memory barriers
                itr->state.store(true, mefdn::memory_order_release);
            }
            
            enqueue_transaction_base::commit();
            
            MEFDN_LOG_DEBUG(
                "msg:Finished enqueuing entry.\t"
                "from:{}\t"
                "to:{}\t"
            ,   this->data_.from
            ,   this->data_.to
            );
        }
    };
    
    enqueue_transaction try_enqueue(const index_type num)
    {
        auto& self = this->derived();
        const auto cap = self.capacity();
        
        auto tail = this->tail_.load(mefdn::memory_order_relaxed);
        
        while (true)
        {
            const auto head = this->head_.load(mefdn::memory_order_acquire);
            
            MEFDN_MAYBE_UNUSED
            const auto old_tail = tail;
            
            const auto new_tail = tail + num;
            const auto new_cap = new_tail - head;
            
            if (MEFDN_UNLIKELY(new_cap >= cap)) {
                return {};
            }
            
            // Try to store the next tail.
            
            // It is dangerous when other producers return the same tail,
            // but in that case this CAS fails
            // because loading "tail_" must precede this CAS (constrained by memory_order_acquire)
            // and other producers must have already written the incremented value.
            
            const bool success =
                this->tail_.compare_exchange_weak(
                    tail
                ,   new_tail
                ,   mefdn::memory_order_acquire
                ,   mefdn::memory_order_relaxed
                );
            
            if (MEFDN_LIKELY(success))
            {
                MEFDN_LOG_DEBUG(
                    "msg:Started enqueuing entry.\t"
                    "head:{}\t"
                    "tail:{}\t"
                    "old_tail:{}\t"
                    "new_tail:{}"
                ,   head
                ,   tail
                ,   old_tail
                ,   new_tail
                );
                
                return enqueue_transaction(derived(), { tail, new_tail });
            }
            else {
                MEFDN_LOG_DEBUG(
                    "msg:Failed CAS to enqueue entry.\t"
                    "head:{}\t"
                    "tail:{}\t"
                    "old_tail:{}\t"
                    "new_tail:{}"
                ,   head
                ,   tail
                ,   old_tail
                ,   new_tail
                );
            }
        }
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
            // Important: num might be 0
            
            MEFDN_ASSERT(this->valid());
            
            MEFDN_ASSERT(num <= (this->data_.to - this->data_.from));
            
            auto itr = this->entry_begin();
            MEFDN_MAYBE_UNUSED const auto last = this->entry_end();
            
            for (index_type i = 0; i < num; ++i, ++itr)
            {
                MEFDN_ASSERT(itr != last);
                
                // The memory barrier is done when head_ is replaced.
                // Hence we use relaxed memory ordering here.
                itr->state.store(false, mefdn::memory_order_relaxed);
            }
            
            auto& self = *this->self_;
            
            const auto new_head = this->data_.from + num;
            
            // Allow producers to use the element at "head".
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.head_.store(new_head, mefdn::memory_order_release);
            
            MEFDN_LOG_DEBUG(
                "msg:Finished dequeuing MPSC queue.\t"
                "from:{}\t"
                "to:{}\t"
                "new_head:{}"
            ,   this->data_.from
            ,   this->data_.to
            ,   new_head
            );
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        const auto head = this->head_.load(mefdn::memory_order_relaxed);
        const auto tail = this->tail_.load(mefdn::memory_order_relaxed);
        
        auto itr = this->make_entry_iterator_at(head);
        const auto last = this->make_entry_iterator_at(tail);
        
        index_type num_dequeued = 0;
        
        for ( ; itr != last; ++itr)
        {
            const bool is_visible = itr->state.load(mefdn::memory_order_acquire);
            
            if (MEFDN_UNLIKELY(!is_visible))
                break;
            
            if (++num_dequeued >= num)
                break;
        }
        
        if (num_dequeued == 0)
            return {};
        else {
            MEFDN_LOG_DEBUG(
                "msg:Started dequeuing MPSC queue.\t"
                "head:{}\t"
                "tail:{}\t"
                "num:{}"
            ,   head
            ,   tail
            ,   num
            );
            
            return dequeue_transaction(derived(), { head, head + num_dequeued });
        }
    }
    size_t peek_num_entries()
    {
        return 
            this->tail_.load(mefdn::memory_order_relaxed)
            - this->head_.load(mefdn::memory_order_relaxed);
    }
    
private:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mefdn
} // namespace menps

