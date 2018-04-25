
#pragma once

#include <menps/mecom2/rma/alltoall_buffer.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>

namespace menps {
namespace mecom2 {

template <typename Elem>
class mpi_alltoall_buffer;

template <typename Elem>
struct mpi_alltoall_buffer_policy
{
    using derived_type = mpi_alltoall_buffer<Elem>;
    
    using rma_itf_type = mpi_rma;
    
    using proc_id_type = mpi_rma_policy::proc_id_type;
    using size_type = mpi_rma_policy::size_type;
    
    using element_type = Elem;
};


template <typename Elem>
class mpi_alltoall_buffer
    : public alltoall_buffer<mpi_alltoall_buffer_policy<Elem>>
{ };

} // namespace mecom2
} // namespace menps

