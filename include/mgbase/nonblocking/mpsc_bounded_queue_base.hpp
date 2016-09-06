
#pragma once

#include "bounded_queue_counter.hpp"

namespace mgbase {

namespace detail {

template <typename Element>
struct mpsc_bounded_queue_entry
{
    Element                 elem;
    mgbase::atomic<bool>    state;
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
    mpsc_bounded_queue_base() MGBASE_DEFAULT_NOEXCEPT = default;
    
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
            : enqueue_transaction_base(self, data)
            { }
        
        enqueue_transaction(const enqueue_transaction&) = delete;
        
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        enqueue_transaction(enqueue_transaction&&) MGBASE_DEFAULT_NOEXCEPT = default;
        #else
        enqueue_transaction(enqueue_transaction&& other) MGBASE_NOEXCEPT
            : enqueue_transaction_base(mgbase::move(other)) { }
        #endif
        
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
        }
    };
    
    enqueue_transaction try_enqueue(const index_type num)
    {
        index_type tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        const index_type cap = derived().capacity();
        
        while (true)
        {
            const index_type head =
                this->head_.load(mgbase::memory_order_acquire);
            
            const index_type new_tail = tail + num;
            
            const index_type new_cap = new_tail - head;
            
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
            
            // Now head_ is written to head by compare_exchange_weak.
            
            if (MGBASE_LIKELY(success))
            {
                return enqueue_transaction(derived(), { tail, new_tail });
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
            : dequeue_transaction_base(self, data) { }
        
        dequeue_transaction(const dequeue_transaction&) = delete;
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        dequeue_transaction(dequeue_transaction&&) MGBASE_DEFAULT_NOEXCEPT = default;
        #else
        dequeue_transaction(dequeue_transaction&& other) MGBASE_NOEXCEPT
            : dequeue_transaction_base(mgbase::move(other)) { }
        #endif
        
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
            
            const auto new_head = this->data_.from + num;
            
            // Allow producers to use the element at "head".
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.head_.store(new_head, mgbase::memory_order_release);
            
            MGBASE_LOG_VERBOSE(
                "msg:Finished dequeuing.\t"
            );
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        const auto head = this->head_.load(mgbase::memory_order_relaxed);
        const auto tail = this->tail_.load(mgbase::memory_order_relaxed);
        
        auto itr = this->make_entry_iterator_at(head);
        const auto last = this->make_entry_iterator_at(tail);
        
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
        else
            return dequeue_transaction(derived(), { head, head + num_dequeued });
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mgbase

