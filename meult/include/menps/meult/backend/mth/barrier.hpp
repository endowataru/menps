
#pragma once

#include "mth.hpp"

namespace menps {
namespace meult {
namespace backend {
namespace mth {

class barrier
{
public:
    explicit barrier(const mefdn::ptrdiff_t num_threads)
    {
        // GCC 4.8 cannot initialize variable with {} ?
        myth_barrierattr_t attr = myth_barrierattr_t();
        myth_barrierattr_init(&attr);
        
        myth_barrier_init(&this->bar_, &attr,
            static_cast<unsigned int>(num_threads));
        
        myth_barrierattr_destroy(&attr);
    }
    
    barrier(const barrier&) = delete;
    barrier& operator = (const barrier&) = delete;
    
    ~barrier()
    {
        if (myth_barrier_destroy(&this->bar_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void arrive_and_wait() noexcept
    {
        if (myth_barrier_wait(&this->bar_) != 0) {
            // We cannot throw exception from this method
            // (decided by the standard draft).
        }
    }
    
private:
    myth_barrier_t bar_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

