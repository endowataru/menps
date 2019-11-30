
#pragma once

#include <menps/mefdn/logger.hpp>
#include <signal.h>
#include <ucontext.h>

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

inline fault_kind_t get_fault_kind(const ucontext_t* const uc)
{
    // See also: https://stackoverflow.com/questions/43033177/

    #ifdef __x86_64__
    // TODO: This assumes x86-64.
    const auto reg_err_val = uc->uc_mcontext.gregs[REG_ERR];
    if (reg_err_val & (1 << 4 /* == X86_PF_INSTR */)) {
        return fault_kind_t::exec;
    }
    else if (reg_err_val & (1 << 1 /* == X86_PF_WRITE */)) {
        return fault_kind_t::write;
    }
    else {
        return fault_kind_t::read;
    }
    #else
    return fault_kind_t::unknown;
    #endif
}

} // namespace medsm2
} // namespace menps

