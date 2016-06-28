
#include "fjmpi_delegator.hpp"
#include "common/command/delegate.hpp"
#include "device/fjmpi/fjmpi_error.hpp"

namespace mgcom {
namespace fjmpi {

fjmpi_delegator::fjmpi_delegator(command_producer& cp)
    : cp_(cp) { }

bool fjmpi_delegator::try_get_async(const get_params& params)
{
    struct closure
    {
        const get_params& params;
        
        void operator() (fjmpi::get_params* const dest) const
        {
            *dest = params;
        }
    };
    
    return cp_.try_enqueue<get_params>(
        command_code::fjmpi_put
    ,   closure{params}
    );
}

bool fjmpi_delegator::try_put_async(const put_params& params)
{
    struct closure
    {
        const put_params& params;
        
        void operator() (fjmpi::put_params* const dest) const
        {
            *dest = params;
        }
    };
    
    return cp_.try_enqueue<put_params>(
        command_code::fjmpi_put
    ,   closure{params}
    );
}
#if 0

bool fjmpi_delegator::try_get_async(const get_params& params)
{
    struct MGBASE_ALIGNAS(8) closure
    {
        get_params          params;
        fjmpi_completer*    comp;
        
        bool operator() () const
        {
            const int nic = get_local_nic_from_flag(params.flags);
            
            int tag;
            const bool found_tag = comp->try_new_tag(params.proc, nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
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
                
                comp->set_notification(params.proc, nic, tag, params.on_complete);
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
    };
    
    const auto ret = try_delegate(del_, closure{ params, &comp_ });
    
    MGBASE_LOG_DEBUG(
        "msg:{}\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
    ,   (ret ? "Delegated FJMPI_Rdma_get." : "Failed to delegate FJMPI_Rdma_get.")
    ,   params.proc
    ,   params.laddr
    ,   params.raddr
    ,   params.size_in_bytes
    ,   params.flags
    );
    
    return ret;
}

bool fjmpi_delegator::try_put_async(const put_params& params)
{
    struct MGBASE_ALIGNAS(8) closure
    {
        put_params          params;
        fjmpi_completer*    comp;
        
        bool operator() () const
        {
            const int nic = get_local_nic_from_flag(params.flags);
            
            int tag;
            const bool found_tag = comp->try_new_tag(params.proc, nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
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
                
                comp->set_notification(params.proc, nic, tag, params.on_complete);
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
    };
    
    const auto ret = try_delegate(del_, closure{ params, &comp_ });
    
    MGBASE_LOG_DEBUG(
        "msg:{}\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
    ,   (ret ? "Delegated FJMPI_Rdma_put." : "Failed to delegate FJMPI_Rdma_put.")
    ,   params.proc
    ,   params.laddr
    ,   params.raddr
    ,   params.size_in_bytes
    ,   params.flags
    );
    
    return ret;
}

#endif

reg_mem_result fjmpi_delegator::reg_mem(const reg_mem_params& params)
{
    struct closure
    {
        reg_mem_params  params;
        memid_pool*     pool;
        reg_mem_result* res;
        
        MGBASE_WARN_UNUSED_RESULT
        bool operator () () const
        {
            int memid;
            if (MGBASE_UNLIKELY(!pool->try_allocate(&memid)))
                return false;
            
            const mgbase::uint64_t laddr =
                fjmpi_error::assert_not_error(
                    FJMPI_Rdma_reg_mem(
                        memid
                    ,   params.buf
                    ,   params.length
                    )
                );
            
            MGBASE_LOG_DEBUG(
                "msg:Registered region.\t"
                "ptr:{:x}\tsize:{}\tladdr:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(params.buf)
            ,   params.length
            ,   laddr
            );
            
            *res = { memid, laddr };
            
            return true;
        }
    };
    
    reg_mem_result res;
    
    execute(cp_.get_delegator(), closure{ params, &pool_, &res });
    
    return res;
}

get_remote_addr_result fjmpi_delegator::get_remote_addr(const get_remote_addr_params& params)
{
    struct closure
    {
        get_remote_addr_params  params;
        get_remote_addr_result* result;
        
        bool operator () () const
        {
            const mgbase::uint64_t raddr =
                fjmpi_error::assert_not_error(
                    FJMPI_Rdma_get_remote_addr(params.pid, params.memid)
                );
            
            MGBASE_LOG_DEBUG(
                "msg:Got remote address.\tproc:{}\tmemid:{}\traddr:{:x}"
            ,   params.pid
            ,   params.memid
            ,   raddr
            );
            
            *result = { raddr };
            
            return true;
        }
    };
    
    get_remote_addr_result result;
    
    execute(cp_.get_delegator(), closure{ params, &result });
    
    return result;
}

void fjmpi_delegator::dereg_mem(const dereg_mem_params& params)
{
    struct closure
    {
        dereg_mem_params    params;
        memid_pool*         pool;
        
        bool operator () () const
        {
            fjmpi_error::assert_zero(
                FJMPI_Rdma_dereg_mem(params.memid)
            );

            MGBASE_LOG_DEBUG(
                "msg:Deregistered region.\tmemid:{}"
            ,   params.memid
            );
            
            pool->deallocate(params.memid);
            
            return true;
        }
    };
    
    execute(cp_.get_delegator(), closure{ params, &pool_ });
}

} // namespace fjmpi
} // namespace mgcom

