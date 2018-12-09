
#pragma once

#include "device/fjmpi/fjmpi_interface.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

namespace detail {

inline int get_local_nic_from_flag(const int flags)
{
    MEFDN_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC0 == 0);
    MEFDN_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC1 == 1);
    MEFDN_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC2 == 2);
    MEFDN_STATIC_ASSERT(FJMPI_RDMA_LOCAL_NIC3 == 3);
    
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
        default:                    MEFDN_UNREACHABLE();
    }*/
}

} // namespace detail

template <typename Completer>
inline bool try_execute_get(const get_params& params, Completer& completer)
{
    const int nic = detail::get_local_nic_from_flag(params.flags);
    
    int tag;
    const bool found_tag = completer.try_new_tag(params.proc, nic, &tag);
    
    if (MEFDN_LIKELY(found_tag))
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_get(
                params.proc
            ,   tag
            ,   params.raddr
            ,   params.laddr
            ,   params.size_in_bytes
            ,   params.flags
            )
        );
        
        completer.set_notification(params.proc, nic, tag, params.on_complete);
    }
    
    MEFDN_LOG_DEBUG(
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

template <typename Completer>
inline bool try_execute_put(const put_params& params, Completer& completer)
{
    const int nic = detail::get_local_nic_from_flag(params.flags);
    
    int tag;
    const bool found_tag = completer.try_new_tag(params.proc, nic, &tag);
    
    if (MEFDN_LIKELY(found_tag))
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_put(
                params.proc
            ,   tag
            ,   params.raddr
            ,   params.laddr
            ,   params.size_in_bytes
            ,   params.flags
            )
        );
        
        completer.set_notification(params.proc, nic, tag, params.on_complete);
    }
    
    MEFDN_LOG_DEBUG(
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

#define MECOM_FJMPI_COMMAND_EXECUTE_CASES(CASE, RETURN, params, completer) \
    CASE(fjmpi_get) { \
        MEFDN_STATIC_ASSERT(sizeof(get_params) <= sizeof(params)); \
        const bool ret = ::mecom::fjmpi::try_execute_get(reinterpret_cast<const get_params&>(params), (completer)); \
        RETURN(ret) \
    } \
    CASE(fjmpi_put) { \
        MEFDN_STATIC_ASSERT(sizeof(put_params) <= sizeof(params)); \
        const bool ret = ::mecom::fjmpi::try_execute_put(reinterpret_cast<const put_params&>(params), (completer)); \
        RETURN(ret) \
    }

} // namespace fjmpi
} // namespace mecom
} // namespace menps
