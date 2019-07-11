
#pragma once

#include <menps/medev2/mpi/mpi.hpp>
#include <menps/mefdn/type_traits/integral_constant.hpp>

namespace menps {
namespace medev2 {
namespace mpi {

template <typename T>
struct get_datatype;

template <> struct get_datatype<unsigned long> {
    MEFDN_STATIC_ASSERT(sizeof(unsigned long) == 8);
    MPI_Datatype operator() () const noexcept { return MPI_UINT64_T; }
};

template <> struct get_datatype<unsigned long long> {
    MEFDN_STATIC_ASSERT(sizeof(unsigned long long) == 8);
    MPI_Datatype operator() () const noexcept { return MPI_UINT64_T; }
};

} // namespace mpi
} // namespace medev2
} // namespace menps

