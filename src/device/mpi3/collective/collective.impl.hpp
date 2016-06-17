
#pragma once

#include "device/mpi3/command/mpi3_command_queue_base.hpp"
#include <mgcom/collective.hpp>
#include "collective.hpp"

namespace mgcom {
namespace mpi3 {

// Points to a global variable defined in another compilation unit.
//  /*???*/& g_queue;

namespace collective {

namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    virtual ~mpi3_requester() MGBASE_EMPTY_DEFINITION
    
    virtual void barrier()
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (!mgcom::mpi3::g_queue.try_ibarrier(
            MPI_COMM_WORLD // TODO
        ,   mgbase::make_operation_store_release(&flag, true)
        ))
        {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void broadcast(const untyped::broadcast_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (!mgcom::mpi3::g_queue.try_ibcast(
            params.root
        ,   params.ptr
        ,   params.num_bytes
        ,   MPI_COMM_WORLD // TODO
        ,   mgbase::make_operation_store_release(&flag, true)
        ))
        {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void allgather(const untyped::allgather_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (!mgcom::mpi3::g_queue.try_iallgather(
            params.src
        ,   params.dest
        ,   params.num_bytes
        ,   MPI_COMM_WORLD // TODO
        ,   mgbase::make_operation_store_release(&flag, true)
        ))
        {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void alltoall(const untyped::alltoall_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (!mgcom::mpi3::g_queue.try_ialltoall(
            params.src
        ,   params.dest
        ,   params.num_bytes
        ,   MPI_COMM_WORLD // TODO
        ,   mgbase::make_operation_store_release(&flag, true)
        ))
        {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    // TODO : use make_unique
    return mgbase::unique_ptr<requester>(new mpi3_requester);
}

} // namespace collective

} // namespace mpi3
} // namespace mgcom

