
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/atomic.hpp>

namespace mgbase {

template <typename T, std::size_t N>
class static_bounded_index_queue
    : noncopyable
{
    MGBASE_STATIC_ASSERT_MSG((N & (N - 1)) == 0, "N must be a power of 2"); // TODO : remove the limitation
    static const T tag_step   = N;
    static const T value_mask = N - 1;
    
public:
    static_bounded_index_queue()
        : first_(0)
        , last_(0)
    {
        for (std::size_t i = 0; i < N; i++)
            elements_[i].store(i, memory_order_relaxed);
    }
    
    void enqueue(T index) {
        // elements_[index] has been allowed for this thread to modify.
        asm volatile ("# enqueue started");
        
        elements_[index].store(index, memory_order_relaxed);
        
        T prev = last_.load(memory_order_relaxed);
        
        while (true) {
            const T target = ((prev & ~value_mask) + tag_step) | index;
            if (last_.compare_exchange_weak(prev, target, memory_order_relaxed)) {
                const T prev_val = prev & value_mask;
                elements_[prev_val].store(index, memory_order_relaxed);
                asm volatile ("# enqueue finished");
                break;
            }
        }
    }
    
    bool try_dequeue(T& result) {
        asm volatile ("# try_dequeue started");
        
        T target = first_.load(memory_order_relaxed);
        while (true) {
            const T target_val = target & value_mask;
            const T next_val = elements_[target_val].load(memory_order_relaxed);
            
            if (next_val == target_val) {
                asm volatile ("# try_dequeue failed");
                return false;
            }
            
            const T next = ((target & ~value_mask) + tag_step) | next_val;
            
            if (first_.compare_exchange_weak(target, next, memory_order_relaxed)) {
                result = target_val;
                asm volatile ("# try_dequeue succeeded");
                return true;
            }
            
            // target loaded first_
        }
        
    }

private:
    atomic<T> elements_[N];
    atomic<T> first_;
    atomic<T> last_;
};

}


