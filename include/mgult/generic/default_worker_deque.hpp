
#pragma once

#include <mgult/ult_id.hpp>
#include <mgbase/atomic.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgult {

class worker_deque_error
    : public std::exception { };

class default_worker_deque
{
    typedef mgbase::int32_t     index_type;
    
public:
    explicit default_worker_deque(mgbase::size_t size)
        : top_{0}
        , bottom_{0}
        , array_{new ult_id[size]}
        , size_{size}
    {
        for (mgbase::size_t i = 0; i < size; ++i) {
            array_[i] = make_invalid_ult_id();
        }
    }
    
    ult_id try_pop_top() // take in Chase-Lev deque, pop in Cilk
    {
        const auto b = bottom_.load(mgbase::memory_order_relaxed) - 1;
        const auto a = array_.get();
        
        bottom_.store(b, mgbase::memory_order_relaxed);
        
        // Read/write barrier.
        mgbase::atomic_thread_fence(mgbase::memory_order_seq_cst);
        
        auto t = top_.load(mgbase::memory_order_relaxed);
        
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
                ,   mgbase::memory_order_seq_cst
                ,   mgbase::memory_order_relaxed
                ))
                {
                    x = make_invalid_ult_id();
                }
                
                bottom_.store(b + 1, mgbase::memory_order_relaxed);
            }
        }
        else {
            bottom_.store(b + 1, mgbase::memory_order_relaxed);
        }
        
        return x;
    }
    
    void push_top(const ult_id id) // fork
    {
        const auto b = bottom_.load(mgbase::memory_order_relaxed);
        const auto t = top_.load(mgbase::memory_order_acquire);
        const auto a = array_.get();
        
        if (MGBASE_UNLIKELY( b - t > size_ - 1 )) {
            throw worker_deque_error{};
        }
        
        a[b % size_] = id;
        
        mgbase::atomic_thread_fence(mgbase::memory_order_release);
        
        bottom_.store(b + 1, mgbase::memory_order_relaxed);
    }
    
    void push_bottom(const ult_id id) {
        // TODO
        push_top(id);
    }
    
    ult_id try_pop_bottom() // steal
    {
        /*const*/ auto t = top_.load(mgbase::memory_order_acquire);
        
        mgbase::atomic_thread_fence(mgbase::memory_order_seq_cst);
        
        const auto b = bottom_.load(mgbase::memory_order_acquire);
        
        auto x = make_invalid_ult_id();
        
        if (t < b) {
            const auto a = array_.get();
            
            x = a[t % size_];
            
            if (! top_.compare_exchange_strong(
                t
            ,   t + 1
            ,   mgbase::memory_order_seq_cst
            ,   mgbase::memory_order_relaxed
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
    mgbase::atomic<index_type>      top_;
    mgbase::atomic<index_type>      bottom_;
    mgbase::unique_ptr<ult_id []>   array_;
    mgbase::size_t                  size_;
};

} // namespace mgult

