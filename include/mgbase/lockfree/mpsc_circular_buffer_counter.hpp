
#pragma once

#include <mgbase/threading/atomic.hpp>

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
    bool try_enqueue(Index* const tail_result)
    {
        while (true)
        {
            // Load "tail" and "head" with normal load.
            /*const*/ Index tail = tail_.load(mgbase::memory_order_relaxed);
                // const is removed for compare_exchange_weak
            const Index head = head_.load(mgbase::memory_order_relaxed);
            
            // Increment "tail".
            // If the index is a constant expression and a power of 2,
            // then a remainder operation is expected to be optimized as a logical AND operation.
            const Index next_tail = (tail + 1) % derived().size();
            
            // Check if the buffer is full.
            if (head == next_tail) {
                // The buffer is full.
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
                *tail_result = tail;
                
                return true;
            }
        }
        
    }
    
    bool try_start_dequeue(Index* const head_result)
    {
        // Load "tail" and "head" with normal load.
        const Index tail = tail_.load(mgbase::memory_order_relaxed);
        const Index head = head_.load(mgbase::memory_order_relaxed);
        
        // Check if the buffer is empty.
        if (tail == head) {
            // The buffer is empty.
            return false;
        }
        
        // Successfully started dequeuing.
        *head_result = head;
        
        return true;
    }
    
    void finish_dequeue(const Index head)
    {
        // Increment "head".
        const Index next_head = (head + 1) % derived().size();
        
        // Allow producers to use the element at "head".
        // If a producer sees this modification,
        // then processing the element on the current thread must have completed.
        head_.store(next_head, mgbase::memory_order_release);
    }
    
protected:
    mpsc_circular_buffer_counter_base() { }
    
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    
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

