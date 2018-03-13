
#pragma once

#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>

namespace menps {
namespace mefdn {

class barrier
{
public:
    explicit barrier(const mefdn::ptrdiff_t num_threads)
        : num_(num_threads)
    { }
    
    barrier(const barrier&) = delete;
    barrier& operator = (const barrier&) = delete;
    
    void arrive_and_wait() noexcept
    {
        mefdn::unique_lock<mefdn::mutex> lk(this->mtx_);
        
        const auto gen = this->gen_;
        
        if (++this->cur_ < this->num_) {
            cond_.wait(lk,
                [this, gen] {
                    return gen != this->gen_;
                });
        }
        else {
            ++this->gen_;
            this->cur_ = 0;
            cond_.notify_all();
        }
    }
    
private:
    mefdn::ptrdiff_t cur_ = 0;
    mefdn::ptrdiff_t num_ = 0;
    mefdn::ptrdiff_t gen_ = 0;
    mefdn::mutex mtx_;
    mefdn::condition_variable cond_;
};

} // namespace mefdn
} // namespace menps

