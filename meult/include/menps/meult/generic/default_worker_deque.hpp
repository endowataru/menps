
#pragma once

#include <menps/meult/ult_id.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {

class worker_deque_error
    : public std::exception { };

class default_worker_deque
{
    typedef mefdn::int32_t     index_type;
    
public:
    explicit default_worker_deque(mefdn::size_t size)
        : top_{0}
        , bottom_{0}
        , array_{new ult_id[size]}
        , size_{static_cast<index_type>(size)}
    {
        for (mefdn::size_t i = 0; i < size; ++i) {
            array_[i] = make_invalid_ult_id();
        }
    }
    
    ult_id try_pop_top() // take in Chase-Lev deque, pop in Cilk
    {
        const auto b = bottom_.load(mefdn::memory_order_relaxed) - 1;
        const auto a = array_.get();
        
        bottom_.store(b, mefdn::memory_order_relaxed);
        
        // Read/write barrier.
        mefdn::atomic_thread_fence(mefdn::memory_order_seq_cst);
        
        auto t = top_.load(mefdn::memory_order_relaxed);
        
        auto x = make_invalid_ult_id();
        
        //if (t <= b)
        if (b - t >= 0) // This differs from other implementations
        {
            x = a[b % size_];
            
            if (t == b)
            {
                if (! top_.compare_exchange_strong(
                    t
                ,   t + 1
                ,   mefdn::memory_order_seq_cst
                ,   mefdn::memory_order_relaxed
                ))
                {
                    x = make_invalid_ult_id();
                }
                
                bottom_.store(b + 1, mefdn::memory_order_relaxed);
            }
        }
        else {
            bottom_.store(b + 1, mefdn::memory_order_relaxed);
        }
        
        return x;
    }
    
    void push_top(const ult_id id) // fork
    {
        const auto b = bottom_.load(mefdn::memory_order_relaxed);
        const auto t = top_.load(mefdn::memory_order_acquire);
        const auto a = array_.get();
        
        if (MEFDN_UNLIKELY( b - t > size_ - 1 )) {
            throw worker_deque_error{};
        }
        
        a[b % size_] = id;
        
        mefdn::atomic_thread_fence(mefdn::memory_order_release);
        
        bottom_.store(b + 1, mefdn::memory_order_relaxed);
    }
    
    void push_bottom(const ult_id id) {
        // TODO
        push_top(id);
    }
    
    ult_id try_pop_bottom() // steal
    {
        /*const*/ auto t = top_.load(mefdn::memory_order_acquire);
        
        mefdn::atomic_thread_fence(mefdn::memory_order_seq_cst);
        
        const auto b = bottom_.load(mefdn::memory_order_acquire);
        
        auto x = make_invalid_ult_id();
        
        if (t < b) {
            const auto a = array_.get();
            
            x = a[t % size_];
            
            if (! top_.compare_exchange_strong(
                t
            ,   t + 1
            ,   mefdn::memory_order_seq_cst
            ,   mefdn::memory_order_relaxed
            ))
            {
                // Failed race.
                // TODO: EMPTY != ABORT
                x = make_invalid_ult_id();
            }
        }
        
        return x;
    }
    
private:
    mefdn::atomic<index_type>      top_;
    mefdn::atomic<index_type>      bottom_;
    mefdn::unique_ptr<ult_id []>   array_;
    index_type                      size_;
};

} // namespace meult
} // namespace menps

