
#pragma once

#include <mgbase/logger.hpp>
#include <signal.h>

namespace mgdsm {

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
        MGBASE_LOG_FATAL(
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

} // namespace mgdsm

