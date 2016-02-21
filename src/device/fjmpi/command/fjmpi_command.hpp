
#pragma once

#include "fjmpi_completer.hpp"
#include "device/mpi/command/mpi_command.hpp"

namespace mgcom {
namespace fjmpi {

enum fjmpi_command_code
{
    FJMPI_COMMAND_GET = mpi::MPI_COMMAND_END
,   FJMPI_COMMAND_PUT
,   FJMPI_COMMAND_END
};

union fjmpi_command_parameters
{
    mpi::mpi_command_parameters mpi1;
    
    struct contiguous_parameters {
        int                 proc;
        mgbase::uint64_t    laddr;
        mgbase::uint64_t    raddr;
        std::size_t         size_in_bytes;
        int                 flags;
        mgbase::operation   on_complete;
    }
    contiguous;
};

namespace detail {

MGBASE_ALWAYS_INLINE int get_local_nic_from_flag(const int flags)
{
    MGBASE_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC0 == 0);
    MGBASE_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC1 == 1);
    MGBASE_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC2 == 2);
    MGBASE_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC3 == 3);
    
    return flags & 0x03;
    
    /*
    const int filter = FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_LOCAL_NIC3;
    
    switch (flags & filter)
    {
        case FJMPI_RDMA_LOCAL_NIC0: return 0;
        case FJMPI_RDMA_LOCAL_NIC1: return 1;
        case FJMPI_RDMA_LOCAL_NIC2: return 2;
        case FJMPI_RDMA_LOCAL_NIC3: return 3;
        default:                    MGBASE_UNREACHABLE();
    }*/
}

} // namespace detail

MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const fjmpi_command_code        code
,   const fjmpi_command_parameters& params
,   fjmpi_completer&                completer
) {
    switch (code)
    {
        case FJMPI_COMMAND_GET: {
            const fjmpi_command_parameters::contiguous_parameters& p = params.contiguous;
            
            const int nic = detail::get_local_nic_from_flag(p.flags);
            
            int tag;
            const bool found_tag = completer.try_new_tag(p.proc, nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
            {
                fjmpi_error::assert_zero(
                    FJMPI_Rdma_get(p.proc, tag, p.raddr, p.laddr, p.size_in_bytes, p.flags)
                );
                
                completer.set_notification(p.proc, nic, tag, p.on_complete);
            }
            
            MGBASE_LOG_DEBUG(
                "msg:{}\t"
                "src_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}\tnic:{}"
            ,   (found_tag ? "Executed FJMPI_Rdma_get." : "RDMA Get because tag capacity exceeded.")
            ,   p.proc
            ,   p.laddr
            ,   p.raddr
            ,   p.size_in_bytes
            ,   p.flags
            ,   nic
            );
            
            return found_tag;
        }
        
        case FJMPI_COMMAND_PUT: {
            const fjmpi_command_parameters::contiguous_parameters& p = params.contiguous;
            
            const int nic = detail::get_local_nic_from_flag(p.flags);
            
            int tag;
            const bool found_tag = completer.try_new_tag(p.proc, nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
            {
                fjmpi_error::assert_zero(
                    FJMPI_Rdma_put(p.proc, tag, p.raddr, p.laddr, p.size_in_bytes, p.flags)
                );
                
                completer.set_notification(p.proc, nic, tag, p.on_complete);
            }
            
            MGBASE_LOG_DEBUG(
                "msg:{}\t"
                "dest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}\tnic:{}"
            ,   (found_tag ? "Executed FJMPI_Rdma_put." : "RDMA Put failed because tag capacity exceeded.")
            ,   p.proc
            ,   p.laddr
            ,   p.raddr
            ,   p.size_in_bytes
            ,   p.flags
            ,   nic
            );
            
            return found_tag;
        }
        
        default: {
            return mpi::execute_on_this_thread(
                static_cast<mpi::mpi_command_code>(code)
            ,   params.mpi1
            ,   completer.get_mpi1_completer()
            );
        }
    }
}

} // namespace fjmpi
} // namespace mgcom

