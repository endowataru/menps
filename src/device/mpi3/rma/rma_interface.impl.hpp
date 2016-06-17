
#pragma once

#include "rma.hpp"
#include "common/rma/rma.hpp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace mpi3 {

//  /*???*/& g_queue;

namespace rma {

// Points to a global variable defined in another compilation unit.

namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    mpi3_requester()
    {
        // do nothing
    }
    
    virtual ~mpi3_requester()
    {
        // do nothing
    }
    
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_get(
            untyped::to_raw_pointer(params.dest_laddr)
        ,   static_cast<int>(params.src_proc)
        ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.src_raddr))
        ,   static_cast<int>(params.size_in_bytes)
        ,   params.on_complete
        );
    }
    
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_put(
            untyped::to_raw_pointer(params.src_laddr)
        ,   static_cast<int>(params.dest_proc)
        ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.dest_raddr))
        ,   static_cast<int>(params.size_in_bytes)
        ,   params.on_complete
        );
        
    }
    
    virtual bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
            0 // parameter "value" is unused
        ,   params.dest_ptr
        ,   static_cast<int>(params.src_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.src_rptr))
        ,   MPI_NO_OP
        ,   params.on_complete
        );
    }
    
    virtual bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
            params.value
        ,   MGBASE_NULLPTR
        ,   static_cast<int>(params.dest_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.dest_rptr))
        ,   MPI_REPLACE
        ,   params.on_complete
        );
    }
    
    virtual bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_compare_and_swap<atomic_default_t>(
            params.expected
        ,   params.desired
        ,   params.result_ptr
        ,   static_cast<int>(params.target_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
        ,   params.on_complete
        );
    }
    
    virtual bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
            params.value
        ,   params.result_ptr
        ,   static_cast<int>(params.target_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
        ,   MPI_SUM
        ,   params.on_complete
        );
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new mpi3_requester);
}

} // namespace rma
} // namespace mpi3
} // namespace mgcom

