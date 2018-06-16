
#pragma once

#include <menps/mecom2/rma/alltoall_ptr_set.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>

namespace menps {
namespace mecom2 {

// TODO: Remove this header.

template <typename Elem>
using mpi_alltoall_ptr_set =
    alltoall_ptr_set<alltoall_ptr_set_policy<mpi_rma, Elem>>;

} // namespace mecom2
} // namespace menps

