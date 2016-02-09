
#include "collective.hpp"
#include "device/mpi/mpi_call.hpp"

namespace mgcom {
namespace collective {

namespace untyped {

void broadcast(
    const process_id_t  root
,   void* const         ptr
,   const index_t       number_of_bytes
) {
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_broadcast(root, ptr, number_of_bytes);
}

void allgather(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_allgather(src, dest, number_of_bytes);
}

void alltoall(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgcom::collective::barrier();
    
    mgcom::mpi::untyped::native_alltoall(src, dest, number_of_bytes);
}

} // namespace untyped

} // namespace collective
} // namespace mgcom

