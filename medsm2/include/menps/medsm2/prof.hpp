
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/basic_prof.hpp>

namespace menps {
namespace medsm2 {

#define MEDSM2_PROF_KINDS(x) \
    x(omp_barrier) \
    x(barrier) \
    x(fence) \
    x(release) \
    x(release_fast) \
    x(release_tx) \
    x(lock_global) \
    x(begin_tx) \
    x(tx_merge) \
    x(end_tx) \
    x(unlock_global) \
    x(barrier_allgather) \
    x(barrier_acq) \
    x(push_wn) \
    x(start_write_twin) \
    x(tx_read) \
    x(merge_read) \
    x(wn_read) \
    x(read_upgrade) \
    x(write_upgrade)

#define DEFINE_PROF_KIND(name) name,

enum class prof_kind {
    MEDSM2_PROF_KINDS(DEFINE_PROF_KIND)
    end
};

#undef DEFINE_PROF_KIND

struct prof_policy
{
    static const bool is_enabled =
        #ifdef MEDSM2_ENABLE_PROF
            true;
        #else
            false;
        #endif
    
    using prof_kind_type = prof_kind;
    
    static const char* get_name(const prof_kind_type k)
    {
        #define DEFINE_NAME(name)   #name ,
        
        static const char* names[] =
            { MEDSM2_PROF_KINDS(DEFINE_NAME) "end" };
        
        #undef DEFINE_NAME
        
        return names[static_cast<mefdn::size_t>(k)];
    }
};

using prof = mefdn::basic_prof<prof_policy>;

} // namespace medsm2
} // namespace menps

