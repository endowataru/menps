
#pragma once

#include <menps/mefdn/thread/thread.hpp>

namespace menps {
namespace mefdn {

namespace this_thread {

using namespace std::this_thread;

// TODO: Non-standard function
//       We need to implement std::chrono
inline void sleep_for_ns(mefdn::int64_t nsec)
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

} // namespace this_thread

} // namespace mefdn
} // namespace menps

