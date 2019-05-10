
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_cv_barrier
{
private:
    using ptrdiff_type = fdn::ptrdiff_t;
    
    using mutex_type = typename P::mutex_type;
    using cv_type = typename P::cv_type;
    using unique_lock_type = typename P::unique_lock_type;

public:
    explicit basic_cv_barrier(const ptrdiff_type num_threads)
        : num_{num_threads}
    { }
    
    basic_cv_barrier(const basic_cv_barrier&) = delete;
    basic_cv_barrier& operator = (const basic_cv_barrier&) = delete;
    
    void arrive_and_wait() noexcept
    {
        unique_lock_type lk(this->mtx_);
        
        const auto gen = this->gen_;
        
        if (++this->cur_ < this->num_) {
            this->cond_.wait(lk,
                [this, gen] {
                    return gen != this->gen_;
                });
        }
        else {
            ++this->gen_;
            this->cur_ = 0;
            this->cond_.notify_all();
        }
    }
    
private:
    ptrdiff_type cur_ = 0;
    ptrdiff_type num_ = 0;
    ptrdiff_type gen_ = 0;
    mutex_type mtx_;
    cv_type cond_;
};

} // namespace cmpth

