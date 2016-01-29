
#pragma once

#include <mgbase/atomic.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

// Reference:
// http://psy-lob-saw.blogspot.jp/2014/04/notes-on-concurrent-ring-buffer-queue.html

template <
    typename Derived
,   typename Index
>
class mpsc_circular_buffer_counter_base
    : mgbase::noncopyable
{
    // Note: This class doesn't consider ABA problem
    //       because the buffer size is assumed to be large enough
    
public:
    bool try_enqueue(const Index diff, Index* const tail_result)
    {
        MGBASE_ASSERT(diff < derived().size());
        
        while (true)
        {
            // Load "tail" and "head" with normal load.
            /*const*/ Index tail = tail_.load(mgbase::memory_order_relaxed);
                // const is removed for compare_exchange_weak
            const Index head = head_.load(mgbase::memory_order_acquire); // barrier after load
            
            // Increment "tail".
            const Index next_tail = tail + diff;
            
            // Calculate the distance.
            const Index dist = next_tail - head;
            
            // Check if the buffer is full.
            if (dist >= derived().size())
            {
                // The buffer is full.
                MGBASE_LOG_DEBUG(
                    "msg:Queue is full.\thead:{:x}\ttail:{:x}"
                ,   head
                ,   tail
                );
                return false;
            }
            
            // Try to store the next tail.
            
            // It is dangerous when other producers return the same tail,
            // but in that case this CAS fails
            // because loading "tail_" must precede this CAS (constrained by memory_order_acquire)
            // and other producers must have already written the incremented value.
            if (tail_.compare_exchange_weak(tail, next_tail, mgbase::memory_order_acquire))
            {
                // Successfully acquired the element at "tail"; assign the result.
                *tail_result = count_to_index(tail);
                
                MGBASE_LOG_DEBUG(
                    "msg:Enqueued entry.\thead:{:x}\ttail:{:x}\tnext_tail:{:x}"
                ,   head
                ,   tail
                ,   next_tail
                );
                
                return true;
            }
        }
        
    }
    
    bool empty() const
    {
        // Load "tail" and "head".
        // Reordering the following operations must be prohibited
        // because it may allow the execution of dequeuing earlier than enqueuing.
        const Index tail = tail_.load(mgbase::memory_order_relaxed); // TODO: tail is not changed by producers
        const Index head = head_.load(mgbase::memory_order_acquire); // barrier after load
        
        const bool ret = count_to_index(tail) == count_to_index(head);
        return ret;
    }
    
    Index front() const MGBASE_NOEXCEPT
    {
        const Index head = head_.load(mgbase::memory_order_relaxed);
        return count_to_index(head);
    }
    
    void dequeue()
    {
        const Index head = head_.load(mgbase::memory_order_relaxed);
        
        // Increment "head".
        const Index next_head = head + 1;
        
        // Allow producers to use the element at "head".
        // If a producer sees this modification,
        // then processing the element on the current thread must have completed.
        head_.store(next_head, mgbase::memory_order_release);
        
        MGBASE_LOG_DEBUG(
            "msg:Finished dequeuing.\thead:{:x}\tnext_head:{:x}"
        ,   head
        ,   next_head
        );
    }
    
protected:
    mpsc_circular_buffer_counter_base() { }
    
private:
    Index count_to_index(const Index count) const MGBASE_NOEXCEPT
    {
        // If the index is a constant expression and a power of 2,
        // then a remainder operation is expected to be optimized as a logical AND operation.
        return count % derived().size();
    }
    
    const Derived& derived() const MGBASE_NOEXCEPT { return static_cast<const Derived&>(*this); }
    
    mgbase::atomic<Index> head_;
    mgbase::atomic<Index> tail_;
};

template <
    typename Index
,   Index Size
>
class mpsc_static_circular_buffer_counter
    : public mpsc_circular_buffer_counter_base<
        mpsc_static_circular_buffer_counter<Index, Size>
    ,   Index
    >
{
public:
    Index size() const MGBASE_NOEXCEPT { return Size; }
};

} // namespace mgbase

