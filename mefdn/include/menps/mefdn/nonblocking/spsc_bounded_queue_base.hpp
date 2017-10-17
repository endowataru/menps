
#pragma once

#include "bounded_queue_counter.hpp"
#include <menps/mefdn/algorithm.hpp>

namespace menps {
namespace mefdn {

namespace detail {

template <typename Element>
struct spsc_bounded_queue_entry
{
    spsc_bounded_queue_entry()
        : elem{}
        { }
    
    Element                 elem;
};

template <typename Traits>
struct spsc_bounded_queue_traits
    : Traits
{
    typedef typename Traits::element_type           element_type;
    
    typedef spsc_bounded_queue_entry<element_type>  entry_type;
};

template <typename Traits>
class spsc_bounded_queue_base
    : public bounded_queue_counter<spsc_bounded_queue_traits<Traits>>
{
    typedef spsc_bounded_queue_traits<Traits>   traits_type;
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
    spsc_bounded_queue_base() noexcept = default;
    
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
            MEFDN_ASSERT(num <= this->size());
            
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
            : dequeue_transaction_base(self, data) { }
        
        dequeue_transaction(const dequeue_transaction&) = delete;
        dequeue_transaction& operator = (const dequeue_transaction&) = delete;
        
        dequeue_transaction(dequeue_transaction&&) noexcept = default;
        dequeue_transaction& operator = (dequeue_transaction&&) noexcept = default;
        
        void commit(const index_type num)
        {
            // Important: num might be 0
            
            MEFDN_ASSERT(this->valid());
            MEFDN_ASSERT(num <= this->size());
            
            auto& self = *this->self_;
            
            const auto new_head = this->data_.from + num;
            
            // Allow producers to use the element at "head".
            // If a producer sees this modification,
            // then processing the element on the current thread must have completed.
            self.head_.store(new_head, mefdn::memory_order_release);
            
            MEFDN_LOG_VERBOSE(
                "msg:Finished dequeuing.\t"
            );
            
            dequeue_transaction_base::commit();
        }
    };
    
    dequeue_transaction try_dequeue(const index_type num)
    {
        const auto head = this->head_.load(mefdn::memory_order_relaxed);
        const auto tail = this->tail_.load(mefdn::memory_order_relaxed);
        
        const auto new_head = head + num;
        
        const auto to = mefdn::min(new_head, tail);
        
        if (to - head == 0)
            return {};
        else
            return dequeue_transaction(derived(), { head, to });
    }
    
private:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mefdn
} // namespace menps

