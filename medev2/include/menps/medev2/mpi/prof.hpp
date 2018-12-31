
#pragma once

#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <menps/mefdn/basic_prof.hpp>

namespace menps {
namespace medev2 {
namespace mpi {

#define D(dummy, name, Name, tr, num, ...) \
    name,

enum class prof_kind {
    MEDEV2_MPI_FUNCS_ALL(D, /*dummy*/)
    end
};

#undef D

struct prof_policy
{
    static const bool is_enabled =
        #ifdef MEDEV2_ENABLE_PROF
            true;
        #else
            false;
        #endif
    
    using prof_kind_type = prof_kind;
    
    static const char* get_name(const prof_kind_type k)
    {
        #define D(dummy, name, Name, tr, num, ...) \
            #name ,
        
        static const char* names[] =
            { MEDEV2_MPI_FUNCS_ALL(D, /*dummy*/) "end" };
        
        #undef D
        
        return names[static_cast<mefdn::size_t>(k)];
    }
};

using prof = mefdn::basic_prof<prof_policy>;

} // namespace mpi
} // namespace medev2
} // namespace menps

