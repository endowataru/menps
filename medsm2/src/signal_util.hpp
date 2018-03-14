
#pragma once

#include <menps/mefdn/logger.hpp>
#include <signal.h>

namespace menps {
namespace medsm2 {

class signal_error
    : public std::exception { };

inline bool call_sigaction(
    const int                       signum
,   const struct sigaction* const   act
,   struct sigaction* const         oldact
) {
    const int ret = sigaction(signum, act, oldact);
    
    if (ret != 0)
    {
        MEFDN_LOG_FATAL(
            "msg:sigaction() failed.\t"
            "signum:{}"
            "ret:{}\t"
            "errno:{}"
        ,   signum
        ,   ret
        ,   errno
        );
        
        throw signal_error{};
    }
    
    return true;
}

} // namespace medsm2
} // namespace menps

