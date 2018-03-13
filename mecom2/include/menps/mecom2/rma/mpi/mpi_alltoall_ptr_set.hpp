
#pragma once

#include <menps/mecom2/rma/alltoall_ptr_set.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>

namespace menps {
namespace mecom2 {

template <typename Elem>
class mpi_alltoall_ptr_set;

template <typename Elem>
struct mpi_alltoall_ptr_set_policy
{
    using derived_type = mpi_alltoall_ptr_set<Elem>;
    
    using rma_itf_type = mpi_rma;
    
    using process_id_type = mpi_rma_policy::process_id_type; // TODO
    using size_type = mpi_rma_policy::size_type;
    
    using element_type = Elem;
};


template <typename Elem>
class mpi_alltoall_ptr_set
    : public alltoall_ptr_set<mpi_alltoall_ptr_set_policy<Elem>>
{ };

} // namespace mecom2
} // namespace menps

