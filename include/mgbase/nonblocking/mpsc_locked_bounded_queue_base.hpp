
#pragma once

#include "bounded_queue_counter.hpp"
#include <mgbase/crtp_base.hpp>

namespace mgbase {

namespace detail {

template <typename Element>
struct mpsc_bounded_queue_entry
{
    Element                 elem;
    mgbase::atomic<bool>    state;
};

template <typename Policy>
struct mpsc_locked_bounded_queue_policy
    : Policy
{
    typedef typename Policy::element_type           element_type;
    
    typedef mpsc_bounded_queue_entry<element_type>  entry_type;
};

template <typename Policy>
class mpsc_locked_bounded_queue_base
    : public bounded_queue_counter<mpsc_locked_bounded_queue_policy<Policy>>
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef mpsc_locked_bounded_queue_policy<Policy>    policy_type;
    typedef bounded_queue_counter<policy_type>          base;
    
protected:
    typedef typename base::index_type       index_type;
    typedef typename base::element_type     element_type;
    typedef typename base::entry_type       entry_type;
    
    typedef typename base::enqueue_transaction_base enqueue_transaction_base;
    typedef typename base::enqueue_transaction_data enqueue_transaction_data;
    typedef typename base::dequeue_transaction_base dequeue_transaction_base;
    typedef typename base::dequeue_transaction_data dequeue_transaction_data;
    
protected:
    mpsc_locked_bounded_queue_base() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    class enqueue_transaction
        : public enqueue_transaction_base
    {
    public:
        enqueue_transaction() MGBASE_DEFAULT_NOEXCEPT = default;
        
        /*implicit*/ enqueue_transaction(
            derived_type&                   self
        ,   const enqueue_transaction_data& data
        ,   const bool                      sleeping
        ) MGBASE_NOEXCEPT
            : enqueue_transaction_base(self, data)
            , sleeping_(sleeping)
        { }
        
        enqueue_transaction(const enqueue_transaction&) = delete;
        enqueue_transaction& operator = (const enqueue_transaction&) = delete;
        
        MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(enqueue_transaction, enqueue_transaction_base)
        
        void commit(MGBASE_UNUSED const index_type num)
        {
            MGBASE_ASSERT(this->valid());
            
            auto itr = this->entry_begin();
            const auto last = this->entry_end();
            
            MGBASE_ASSERT(num == static_cast<index_type>(last - itr));
            
            for ( ; itr != last; ++itr)
            {
                // TODO: reduce memory barriers
                itr->state.store(true, mgbase::memory_order_release);
            }
            
            enqueue_transaction_base::commit();
            
            MGBASE_LOG_DEBUG(
                "msg:Finished enqueuing entry.\t"
                "from:0x{:x}\t"
                "to:0x{:x}\t"
            ,   this->data_.from
            ,   this->data_.to
            );
        }
        
        bool is_sleeping() const MGBASE_NOEXCEPT {
            return sleeping_;
        }
        
    private:
        bool sleeping_;
    };
    
    enqueue_transaction try_enqueue(const index_type num)
    {
        MGBASE_ASSERT(num > 0);
        
        auto& self = this->derived();
        const auto cap = self.capacity();
        
        auto tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        while (true)
        {
            const auto head = this->head_.load(mgbase::memory_order_acquire);
            
            MGBASE_UNUSED
            const auto old_tail = tail;
            
            // The LSB (Least Significant Bit) of "tail" is used for locking.
            // We don't use head's LSB but it's also shifted to debug easily.
            
            // Add num to tail and also reset the tail's LSB.
            const auto new_tail = (tail + (num << 1)) & ~static_cast<index_type>(0x1);
            const auto new_cap = (new_tail - head) >> 1;
            
            if (MGBASE_UNLIKELY(new_cap >= cap)) {
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
                ,   mgbase::memory_order_acquire
                ,   mgbase::memory_order_relaxed
                );
            
            if (MGBASE_LIKELY(success))
            {
                MGBASE_LOG_DEBUG(
                    "msg:Started enqueuing entry.\t"
                    "head:0x{:x}\t"
                    "tail:0x{:x}\t"
                    "old_tail:0x{:x}\t"
                    "new_tail:0x{:x}"
                ,   head
                ,   tail
                ,   old_tail
                ,   new_tail
                );
                
                return enqueue_transaction(derived(),
                    {
                        (tail >> 1)         // from
                    ,   (new_tail >> 1)     // to
                    }
                    ,   ((tail & 0x1) == 0x1) // sleeping
                        // If the old tail's LSB was "0",
                        // this thread found that the consumer thread is sleeping.
                    );
            }
            else {
                MGBASE_LOG_DEBUG(
                    "msg:Failed CAS to enqueue entry.\t"
                    "head:0x{:x}\t"
                    "tail:0x{:x}\t"
                    "old_tail:0x{:x}\t"
                    "new_tail:0x{:x}"
                ,   head
                ,   tail
                ,   old_tail
                ,   new_tail
                );
                
                // TODO : Avoid contention on CAS
                //        It's better to insert an exponential back-off here
            }
        }
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
        dequeue_transaction& operator = (const dequeue_transaction&) = delete;
        
        MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(dequeue_transaction, dequeue_transaction_base)
        
        void commit(const index_type num)
        {
            // Important: num might be 0
            
            MGBASE_ASSERT(this->valid());
            
            MGBASE_ASSERT(num <= (this->data_.to - this->data_.from));
            
            auto itr = this->entry_begin();
            MGBASE_UNUSED const auto last = this->entry_end();
            
            for (index_type i = 0; i < num; ++i, ++itr)
            {
                MGBASE_ASSERT(itr != last);
                
                // The memory barrier is done when head_ is replaced.
                // Hence we use relaxed memory ordering here.
                itr->state.store(false, mgbase::memory_order_relaxed);
            }
            
            auto& self = *this->self_;
            
            const auto new_head = (this->data_.from + num) << 1;
            
            MGBASE_UNUSED
            const auto old_head = self.head_.load(mgbase::memory_order_relaxed);
            
            // Allow producers to use the element at "head".
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.head_.store(new_head, mgbase::memory_order_release);
            
            MGBASE_LOG_DEBUG(
                "msg:Finished dequeuing MPSC queue.\t"
                "from:0x{:x}\t"
                "to:0x{:x}\t"
                "old_head:0x{:x}\t"
                "new_head:0x{:x}"
            ,   this->data_.from
            ,   this->data_.to
            ,   old_head
            ,   new_head
            );
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        const auto head = this->head_.load(mgbase::memory_order_relaxed);
        const auto tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        auto itr = this->make_entry_iterator_at(head >> 1);
        const auto last = this->make_entry_iterator_at(tail >> 1);
        
        index_type num_dequeued = 0;
        
        for ( ; itr != last; ++itr)
        {
            const bool is_visible = itr->state.load(mgbase::memory_order_acquire);
            
            if (MGBASE_UNLIKELY(!is_visible))
                break;
            
            if (++num_dequeued >= num)
                break;
        }
        
        if (num_dequeued == 0)
            return {};
        else {
            MGBASE_LOG_DEBUG(
                "msg:Started dequeuing MPSC queue.\t"
                "head:0x{:x}\t"
                "tail:0x{:x}\t"
                "num:0x{:x}"
            ,   head
            ,   tail
            ,   num
            );
            
            return dequeue_transaction(derived(),
                { (head >> 1), ((head >> 1) + num_dequeued) });
        }
    }
    
    bool try_sleep()
    {
        const auto head = this->head_.load(mgbase::memory_order_relaxed);
        auto tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        MGBASE_ASSERT((tail & 0x1) == 0x0);
        
        if ((head >> 1) != (tail >> 1)) {
            // The queue is not empty.
            return false;
        }
        
        const auto ret =
            this->tail_.compare_exchange_weak(
                tail
            ,   (tail | 1)
            ,   mgbase::memory_order_relaxed
            ,   mgbase::memory_order_relaxed
            );
        
        MGBASE_LOG_DEBUG(
            "msg:{}.\t"
            "head:0x{:x}\t"
            "tail:0x{:x}\t"
        ,   (ret ? "Succeeded sleeping" : "Falied to sleep")
        ,   head
        ,   tail
        );
        
        return ret;
    }
};

} // namespace detail

} // namespace mgbase

