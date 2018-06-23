
#pragma once

#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>

namespace menps {
namespace meult {
namespace klt {

class emulated_uncond_variable
{
    using unique_lock_type = mefdn::unique_lock<mefdn::mutex>;
    
public:
    void wait()
    {
        unique_lock_type lk(this->mtx_);
        MEFDN_ASSERT(!this->is_ready_);
        while (!this->is_ready_) {
            this->cv_.wait(lk);
        }
        this->is_ready_ = false;
    }
    
    void notify()
    {
        unique_lock_type lk(this->mtx_);
        MEFDN_ASSERT(!this->is_ready_);
        this->is_ready_ = true;
        this->cv_.notify_one();
    }
    
private:
    mefdn::mutex mtx_;
    mefdn::condition_variable cv_;
    bool is_ready_ = false;
};

} // namespace klt
} // namespace meult
} // namespace menps

