
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/basic_prof.hpp>

namespace menps {
namespace meult {

#define MEULT_PROF_KINDS(x) \
    x(myth_barrier_wait) \
    x(myth_uncond_wait) \
    x(myth_uncond_signal) \
    x(myth_uncond_enter) \
    x(myth_uncond_swap) \
    x(myth_uncond_swap_with) \
    x(myth_uncond_wait_with) \
    x(myth_uncond_wait_with_sw) \
    x(qd_progress_count) \
    x(qd_consume_count) \
    x(qd_consume_unlock_succ) \
    x(qd_consume_unlock_fail)

#define D(name) name,

enum class prof_kind { MEULT_PROF_KINDS(D) end };

#undef D

struct prof_policy
{
    static const bool is_enabled =
        #ifdef MEULT_ENABLE_PROF
            true;
        #else
            false;
        #endif
    
    using prof_kind_type = prof_kind;
    
    static const char* get_name(const prof_kind_type k)
    {
        #define D(name) #name ,
        
        static const char* names[] =
            { MEULT_PROF_KINDS(D) "end" };
        
        #undef D
        
        return names[static_cast<mefdn::size_t>(k)];
    }
};

using prof = mefdn::basic_prof<prof_policy>;

} // namespace meult
} // namespace menps

