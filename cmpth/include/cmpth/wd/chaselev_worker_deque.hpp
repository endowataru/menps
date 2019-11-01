
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct worker_deque_error {};

template <typename P>
class chaselev_worker_deque
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using continuation_type = typename P::continuation_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    using task_desc_type = typename P::task_desc_type;
    
    using desc_ptr_type = task_desc_type*;
    using atomic_desc_ptr_type =
        typename base_ult_itf_type::template atomic<desc_ptr_type>;
    
    using size_type = fdn::size_t;
    using ssize_type = fdn::make_signed_t<size_type>;
    using atomic_size_type =
        typename base_ult_itf_type::template atomic<size_type>;
    
    class array
    {
    public:
        explicit array(const size_type size)
            : size_{size}
            , elems_{fdn::make_unique<atomic_desc_ptr_type []>(size)}
        { }

        size_type size() const noexcept { return this->size_; }
        
        desc_ptr_type load(const size_type i) noexcept {
            const auto real_idx = this->get_real_idx(i);
            return this->elems_[real_idx];
        }
        void store(const size_type i, const desc_ptr_type p) noexcept {
            const auto real_idx = this->get_real_idx(i);
            this->elems_[real_idx].store(p, fdn::memory_order_relaxed);
        }
        
    private:
        size_type get_real_idx(const size_type i) const noexcept {
            const auto size = this->size_;
            CMPTH_P_ASSERT(P, (size & (size - 1)) == 0);
            //return i % this->size_;
            return i & (size - 1);
        }

        size_type size_ = 0;
        fdn::unique_ptr<atomic_desc_ptr_type []> elems_;
    };
    
    using atomic_array_ptr_type =
        typename base_ult_itf_type::template atomic<array*>;
    
public:
    chaselev_worker_deque()
        : top_{0}
        , bottom_{0}
        , arr_{new array{P::get_default_deque_size()}}
    { }
    
    ~chaselev_worker_deque() {
        delete this->arr_.load();
        this->arr_ = nullptr;
    }
    
    chaselev_worker_deque(const chaselev_worker_deque&) = delete;
    chaselev_worker_deque& operator = (const chaselev_worker_deque&) = delete;
    
    continuation_type try_local_pop_top() // take in Chase-Lev deque, pop in Cilk
    {
        const auto b = this->bottom_.load(base_ult_itf_type::memory_order_relaxed) - 1;
        const auto a = this->arr_.load(base_ult_itf_type::memory_order_relaxed);
        
        // Decrement the bottom.
        this->bottom_.store(b, base_ult_itf_type::memory_order_relaxed);
        // Read/write barrier.
        base_ult_itf_type::atomic_thread_fence(base_ult_itf_type::memory_order_seq_cst);
        // Load the top.
        auto t = this->top_.load(base_ult_itf_type::memory_order_relaxed);
        
        desc_ptr_type x = nullptr;
        const auto diff = static_cast<ssize_type>(b) - static_cast<ssize_type>(t);
        //if (t <= b) // Original
        if (diff >= 0) // TODO: This differs from other implementations
        {
            x = a->load(b);
            
            if (t == b) {
                if (! this->top_.compare_exchange_strong(t, t + 1,
                    base_ult_itf_type::memory_order_seq_cst,
                    base_ult_itf_type::memory_order_relaxed))
                {
                    // Failed race.
                    x = nullptr;
                }
                this->bottom_.store(b + 1, base_ult_itf_type::memory_order_relaxed);
            }
        }
        else {
            this->bottom_.store(b + 1, base_ult_itf_type::memory_order_relaxed);
        }
        
        return continuation_type{unique_task_ptr_type{x}};
    }
    
    void local_push_top(continuation_type cont) // fork
    {
        const auto b = this->bottom_.load(base_ult_itf_type::memory_order_relaxed);
        const auto t = this->top_.load(base_ult_itf_type::memory_order_acquire);
        const auto a = this->arr_.load(base_ult_itf_type::memory_order_relaxed);
        
        const auto diff = static_cast<ssize_type>(b) - static_cast<ssize_type>(t);
        //if (CMPTH_UNLIKELY( b - t > a->size() - 1 )) // Original
        if (CMPTH_UNLIKELY( diff > static_cast<ssize_type>(a->size() - 1) )) {
            throw worker_deque_error{};
        }
        const desc_ptr_type p = cont.release();
        a->store(b, p);
        
        base_ult_itf_type::atomic_thread_fence(base_ult_itf_type::memory_order_release);
        
        this->bottom_.store(b + 1, base_ult_itf_type::memory_order_relaxed);
    }
    
    void local_push_bottom(continuation_type cont) {
        this->local_push_top(fdn::move(cont)); // TODO
    }
    
    continuation_type try_remote_pop_bottom() // steal
    {
        // Load the top.
        /*const*/ auto t = this->top_.load(base_ult_itf_type::memory_order_acquire);
        // Read/write barrier.
        base_ult_itf_type::atomic_thread_fence(base_ult_itf_type::memory_order_seq_cst);
        // Load the bottom.
        const auto b = this->bottom_.load(base_ult_itf_type::memory_order_acquire);
        
        desc_ptr_type x = nullptr;
        const auto diff = static_cast<ssize_type>(b) - static_cast<ssize_type>(t);
        //if (t < b)
        if (diff > 0)
        {
            const auto a = this->arr_.load(base_ult_itf_type::memory_order_relaxed);
            x = a->load(t);
            
            if (! this->top_.compare_exchange_strong(t, t + 1,
                base_ult_itf_type::memory_order_seq_cst, base_ult_itf_type::memory_order_relaxed))
            {
                // Failed race.
                // TODO: EMPTY != ABORT
                x = nullptr;
            }
        }
        
        return continuation_type{unique_task_ptr_type{x}};
    }
    
private:
    atomic_size_type        top_;
    atomic_size_type        bottom_;
    atomic_array_ptr_type   arr_;
};

} // namespace cmpth

