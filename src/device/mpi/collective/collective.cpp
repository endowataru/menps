
#include "collective.hpp"
#include "device/mpi/mpi_call.hpp"

namespace mgcom {
namespace mpi {
namespace collective {

namespace untyped {

void broadcast(const broadcast_params& params)
{
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_broadcast(
        params.root
    ,   params.ptr
    ,   params.num_bytes
    );
}

void allgather(const allgather_params& params)
{
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_allgather(
        params.src
    ,   params.dest
    ,   params.num_bytes
    );
}

void alltoall(const alltoall_params& params)
{
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_alltoall(
        params.src
    ,   params.dest
    ,   params.num_bytes
    );
}

} // namespace untyped

} // namespace collective
} // namespace mpi
} // namespace mgcom

