
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class kth_worker_set
{
    using worker_num_type = typename P::worker_num_type;
    using worker_num_set_type = typename P::worker_num_set_type;

    using mutex_type = typename P::mutex_type;
    using lock_guard_type = typename P::lock_guard_type;

public:
    explicit kth_worker_set(const worker_num_type num_wks)
        : num_wks_{num_wks}
    {
        for (worker_num_type wk_num = num_wks; wk_num > 0; --wk_num) {
            this->wk_num_set_.push(wk_num);
        }
    }

    kth_worker_set(const kth_worker_set&) = delete;
    kth_worker_set& operator = (const kth_worker_set&) = delete;

    void initialize_worker() {
        CMPTH_P_ASSERT(P, wk_num_ == 0);
        if (CMPTH_UNLIKELY(this->wk_num_set_.empty())) {
            CMPTH_P_LOG_FATAL(P, "Running out thread resources.");
            std::abort();
        }
        {
            lock_guard_type lk{this->mtx_};
            wk_num_ = this->wk_num_set_.top();
            this->wk_num_set_.pop();
        }
        this->assert_valid_wk_num();
    }
    void finalize_worker() {
        this->assert_valid_wk_num();
        {
            lock_guard_type lk{this->mtx_};
            this->wk_num_set_.push(wk_num_);
        }
        wk_num_ = 0;
    }

    worker_num_type get_worker_num() const {
        this->assert_valid_wk_num();
        return wk_num_-1;
    }
    worker_num_type get_num_workers() const {
        return this->num_wks_;
    }

private:
    void assert_valid_wk_num() const {
        CMPTH_P_ASSERT(P, wk_num_ > 0);
        CMPTH_P_ASSERT(P, wk_num_ <= this->num_wks_);
    }

    const worker_num_type num_wks_ = 0;

    mutex_type mtx_;
    worker_num_set_type wk_num_set_;
    
    static thread_local worker_num_type wk_num_;
};

template <typename P>
thread_local typename kth_worker_set<P>::worker_num_type
kth_worker_set<P>::wk_num_ = 0;

} // namespace cmpth

