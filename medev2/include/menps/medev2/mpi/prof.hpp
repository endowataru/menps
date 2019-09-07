
#pragma once

#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <cmpth/prof.hpp>

namespace menps {
namespace medev2 {
namespace mpi {

struct mpi_prof_aspect_policy
{
    #define D(x, name, ...)     x(name)
    #define MPI_PROF_KINDS(x)   MEDEV2_MPI_FUNCS_ALL(D, x)
    
    CMPTH_DEFINE_PROF_ASPECT_POLICY(MPI_PROF_KINDS)
    
    #undef MPI_PROF_KINDS
    #undef D
    
    static constexpr fdn::size_t get_default_mlog_size() noexcept {
        return 1ull << 20;
    }
};

} // namespace mpi
} // namespace medev2
} // namespace menps

