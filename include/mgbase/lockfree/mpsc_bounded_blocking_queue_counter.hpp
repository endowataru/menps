
#pragma once

#include <mgbase/lockfree/mpsc_circular_buffer_counter.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/ult/mutex.hpp>
#include <mgbase/ult/condition_variable.hpp>
#include <mgbase/ult/this_thread.hpp>

namespace mgbase {

template <
    typename Derived
,   typename Index
>
class mpsc_bounded_blocking_queue_counter_base
    : public mpsc_circular_buffer_counter_base<Derived, Index>
{
    typedef mpsc_circular_buffer_counter_base<Derived, Index>   base;
    
    typedef mgbase::ult::mutex                  mutex_type;
    typedef mgbase::ult::condition_variable     condition_variable_type;
    
public:
    mpsc_bounded_blocking_queue_counter_base()
        : wait_flag_{false} { }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(const Index diff, Index* const tail_result) MGBASE_NOEXCEPT
    {
        if (base::try_enqueue(diff, tail_result))
            return true;
        else {
            notify();
            return false;
        }
    }
    
    void notify()
    {
        if (MGBASE_UNLIKELY(wait_flag_.load(mgbase::memory_order_acquire))) {
            mgbase::unique_lock<mutex_type> lc{ mtx_ };
            cv_.notify_one();
        }
    }
    
    using base::number_of_enqueued;
    using base::empty;
    using base::front;
    
    using base::dequeue;
    using base::dequeue_multiple;
    
    void wait()
    {
        mgbase::unique_lock<mutex_type> lc{ mtx_ };
        
        while (MGBASE_UNLIKELY(base::empty())) // TODO: "unlikely" is correct?
        {
            wait_flag_.store(true, mgbase::memory_order_release);
            cv_.wait(lc);
        }
        
        wait_flag_.store(false, mgbase::memory_order_release);
    }
    
private:
    mutex_type              mtx_;
    condition_variable_type cv_;
    mgbase::atomic<bool>    wait_flag_;
};


template <
    typename Index
,   Index Size
>
class mpsc_bounded_blocking_queue_counter
    : public mpsc_bounded_blocking_queue_counter_base<
        mpsc_bounded_blocking_queue_counter<Index, Size>
    ,   Index
    >
{
public:
    Index size() const MGBASE_NOEXCEPT { return Size; }
};

} // namespace mgbase

