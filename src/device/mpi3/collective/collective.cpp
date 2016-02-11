
#include "device/mpi3/command/mpi3_command_queue_base.hpp"

namespace mgcom {

namespace mpi3 {

// Points to a global variable defined in another compilation unit.
extern mpi3_command_queue_base& g_queue;

} // namespace mpi3

namespace collective {

void barrier()
{
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    while (!mgcom::mpi3::g_queue.try_ibarrier(
        MPI_COMM_WORLD // TODO
    ,   mgbase::make_operation_store_release(&flag, true)
    ))
    { }
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
}

namespace untyped {

void broadcast(
    const process_id_t  root
,   void* const         ptr
,   const index_t       number_of_bytes
) {
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    while (!mgcom::mpi3::g_queue.try_ibcast(
        root
    ,   ptr
    ,   number_of_bytes
    ,   MPI_COMM_WORLD // TODO
    ,   mgbase::make_operation_store_release(&flag, true)
    ))
    { }
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
}


void allgather(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    while (!mgcom::mpi3::g_queue.try_iallgather(
        src
    ,   dest
    ,   number_of_bytes
    ,   MPI_COMM_WORLD // TODO
    ,   mgbase::make_operation_store_release(&flag, true)
    ))
    { }
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
}

void alltoall(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    while (!mgcom::mpi3::g_queue.try_ialltoall(
        src
    ,   dest
    ,   number_of_bytes
    ,   MPI_COMM_WORLD // TODO
    ,   mgbase::make_operation_store_release(&flag, true)
    ))
    { }
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
}

} // namespace untyped

} // namespace collective

} // namespace mgcom

