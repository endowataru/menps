
#pragma once

#include <mgbase/threading/thread_id.hpp>
#include <time.h>
#include <errno.h>
#include <sched.h>
#include <stdexcept>

namespace mgbase {

namespace this_thread {

inline thread_id get_id()
{
    const pthread_t th = pthread_self();
    return thread_id(th);
}

// TODO: Non-standard function
//       We need to implement std::chrono
inline void sleep_for_ns(mgbase::int64_t nsec)
{
    timespec req;
    req.tv_sec  = static_cast<time_t>(nsec / 1000000000);
    req.tv_nsec = static_cast<long>(nsec % 1000000000);
    
    while (true)
    {
        timespec rem;
        
        const int ret = nanosleep(&req, &rem);
        if (ret == 0)
            break;
        else {
            if (errno == EINTR) {
                req = rem;
            }
            else
                throw std::runtime_error("nanosleep()");
        }
    }
}

inline void yield()
{
    sched_yield();
}

} // namespace this_thread

} // namespace mgbase

