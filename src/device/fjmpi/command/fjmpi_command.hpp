
#pragma once

#include "fjmpi_completer.hpp"
#include "device/mpi/command/mpi_command.hpp"

//#include <mgbase/profiling/clock.hpp>

#include <mgbase/logging/queueing_logger.hpp>

//#include <mgbase/profiling/average_accumulator.hpp>
//#include <mgbase/profiling/stopwatch.hpp>

//mgbase::average_accumulator<mgbase::cpu_clock_t> g_get_cycles;

namespace mgcom {
namespace fjmpi {

union fjmpi_command_parameters
{
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
    // More portable implementation
    
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

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_get(
    const fjmpi_command_parameters::contiguous_parameters&  params
,   fjmpi_completer&                                        completer
) {
    const int nic = detail::get_local_nic_from_flag(params.flags);
    
    int tag;
    const bool found_tag = completer.try_new_tag(params.proc, nic, &tag);
    
    if (MGBASE_LIKELY(found_tag))
    {
        //std::cout << mgcom::current_process_id() << " " << mgbase::get_cpu_clock() << " get" << std::endl;
        mgbase::queueing_logger::add_log("get start", nic, tag);
        
        /*mgbase::stopwatch sw;
        sw.start();*/
        
        fjmpi_error::assert_zero(
            FJMPI_Rdma_get(params.proc, tag, params.raddr, params.laddr, params.size_in_bytes, params.flags)
        );
        
        //g_get_cycles.add(sw.elapsed());
        
        mgbase::queueing_logger::add_log("get finish", nic, tag);
        
        completer.set_notification(params.proc, nic, tag, params.on_complete);
    }
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "src_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}\tnic:{}"
    ,   (found_tag ? "Executed FJMPI_Rdma_get." : "RDMA Get because tag capacity exceeded.")
    ,   params.proc
    ,   params.laddr
    ,   params.raddr
    ,   params.size_in_bytes
    ,   params.flags
    ,   nic
    );
    
    return found_tag;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_put(
    const fjmpi_command_parameters::contiguous_parameters&  params
,   fjmpi_completer&                                        completer
) {
    const int nic = detail::get_local_nic_from_flag(params.flags);
    
    int tag;
    const bool found_tag = completer.try_new_tag(params.proc, nic, &tag);
    
    if (MGBASE_LIKELY(found_tag))
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_put(params.proc, tag, params.raddr, params.laddr, params.size_in_bytes, params.flags)
        );
        
        completer.set_notification(params.proc, nic, tag, params.on_complete);
    }
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "dest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}\tnic:{}"
    ,   (found_tag ? "Executed FJMPI_Rdma_put." : "RDMA Put failed because tag capacity exceeded.")
    ,   params.proc
    ,   params.laddr
    ,   params.raddr
    ,   params.size_in_bytes
    ,   params.flags
    ,   nic
    );
    
    return found_tag;
}

#define MGCOM_FJMPI_COMMAND_CODES(x)    \
        x(FJMPI_COMMAND_GET)            \
    ,   x(FJMPI_COMMAND_PUT)

#define MGCOM_FJMPI_COMMAND_EXECUTE_CASES(CASE, RETURN, params, completer) \
    CASE(FJMPI_COMMAND_GET): { \
        const bool ret = ::mgcom::fjmpi::try_execute_get((params).contiguous, (completer)); \
        RETURN(ret);\
    } \
    CASE(FJMPI_COMMAND_PUT): { \
        const bool ret = ::mgcom::fjmpi::try_execute_put((params).contiguous, (completer)); \
        RETURN(ret); \
    }

} // namespace fjmpi
} // namespace mgcom

