
#pragma once

#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/rma/address.hpp"
#include "completer.hpp"
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace ibv {

class direct_proxy
    : mgbase::noncopyable
{
public:
    direct_proxy(endpoint& ep, completer& comp)
        : ep_(&ep), completer_(&comp) { }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_remote_read_async(
        const process_id_t&                     src_proc
    ,   const rma::untyped::remote_address&     remote_addr
    ,   const rma::untyped::local_address&      local_addr
    ,   const index_t&                          size_in_bytes
    ,   const mgbase::operation&                on_complete
    ) const
    {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(!completer_->try_complete(on_complete, &wr_id)))
            return false;
        
        const bool ret = ep_->try_read_async(
            wr_id
        ,   src_proc
        ,   to_raddr(remote_addr)
        ,   to_rkey(remote_addr)
        ,   to_laddr(local_addr)
        ,   to_lkey(local_addr)
        ,   size_in_bytes
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_remote_write_async(
        const process_id_t&                 dest_proc
    ,   const rma::untyped::remote_address& remote_addr
    ,   const rma::untyped::local_address&  local_addr
    ,   const index_t&                      size_in_bytes
    ,   const mgbase::operation&            on_complete
    ) const
    {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(!completer_->try_complete(on_complete, &wr_id)))
            return false;
        
        const bool ret = ep_->try_write_async(
            wr_id
        ,   dest_proc
        ,   to_raddr(remote_addr)
        ,   to_rkey(remote_addr)
        ,   to_laddr(local_addr)
        ,   to_lkey(local_addr)
        ,   size_in_bytes
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_remote_compare_and_swap_async(
        const process_id_t&                                     target_proc
    ,   const rma::remote_ptr<rma::atomic_default_t>&       target_ptr
    ,   const rma::local_ptr<const rma::atomic_default_t>&  expected_ptr
    ,   const rma::local_ptr<const rma::atomic_default_t>&  desired_ptr
    ,   const rma::local_ptr<rma::atomic_default_t>&        result_ptr
    ,   const mgbase::operation&                                on_complete
    ) const
    {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(!completer_->try_complete(on_complete, &wr_id)))
            return false;
        
        const rma::untyped::remote_address target_raddr = target_ptr.to_address();
        const rma::untyped::local_address  result_laddr = result_ptr.to_address();
        
        const bool ret = ep_->try_compare_and_swap_async(
            wr_id
        ,   target_proc
        ,   to_raddr(target_raddr)
        ,   to_rkey(target_raddr)
        ,   to_laddr(result_laddr)
        ,   to_lkey(result_laddr)
        ,   *expected_ptr
        ,   *desired_ptr
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_remote_fetch_and_add_async(
        const process_id_t&                                     target_proc
    ,   const rma::remote_ptr<rma::atomic_default_t>&       target_ptr
    ,   const rma::local_ptr<const rma::atomic_default_t>&  value_ptr
    ,   const rma::local_ptr<rma::atomic_default_t>&        result_ptr
    ,   const mgbase::operation&                                on_complete
    ) const {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(!completer_->try_complete(on_complete, &wr_id)))
            return false;
        
        const rma::untyped::remote_address target_raddr = target_ptr.to_address();
        const rma::untyped::local_address  result_laddr = result_ptr.to_address();
        
        const bool ret = ep_->try_fetch_and_add_async(
            wr_id
        ,   target_proc
        ,   to_raddr(target_raddr)
        ,   to_rkey(target_raddr)
        ,   to_laddr(result_laddr)
        ,   to_lkey(result_laddr)
        ,   *value_ptr
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }

private:
    endpoint*   ep_;
    completer*  completer_;
};

} // namespace ibv
} // namespace mgcom

