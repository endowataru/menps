
#pragma once

#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/rma/address.hpp"
#include "completer.hpp"
#include <mgcom/rma/requester.hpp>

#include "device/ibv/native/send_work_request.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include "set_params.hpp"

namespace mgcom {
namespace ibv {

class direct_proxy
    : mgbase::noncopyable
{
public:
    direct_proxy(endpoint& ep, completer& comp)
        : ep_(&ep), completer_(&comp) { }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_read_async(const rma::untyped::read_params& params) const
    {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(
            !completer_->try_complete(params.on_complete, &wr_id)
        )) {
            return false;
        }
        
        read_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_read_params(params, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_read(ibv_params);
        wr.set_read(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        const bool ret = ep_->try_post_send(params.src_proc, wr);
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_write_async(const rma::untyped::write_params& params) const
    {
        mgbase::uint64_t wr_id;
        if (MGBASE_UNLIKELY(
            !completer_->try_complete(params.on_complete, &wr_id)
        )) {
            return false;
        }
        
        write_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_write_params(params, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_write(ibv_params);
        wr.set_write(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        const bool ret = ep_->try_post_send(params.dest_proc, wr);
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_atomic_read_async(const rma::atomic_read_params<rma::atomic_default_t>& params) const
    {
        mgbase::uint64_t wr_id;
        mgcom::rma::local_ptr<rma::atomic_default_t> result_lptr;
        
        if (MGBASE_UNLIKELY(
            !completer_->try_complete_with_result(params.on_complete, &wr_id, params.dest_ptr, &result_lptr)
        )) {
            return false;
        }
        
        read_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_atomic_read_params(params, result_lptr, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_read(ibv_params);
        wr.set_read(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        // TODO: Atomicity of ordinary read
        const bool ret = ep_->try_post_send(params.src_proc, wr);
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_atomic_write_async(const rma::atomic_write_params<rma::atomic_default_t>& params) const
    {
        mgbase::uint64_t wr_id;
        mgcom::rma::local_ptr<rma::atomic_default_t> result_lptr;
        
        if (MGBASE_UNLIKELY(
            !completer_->try_complete_with_result(params.on_complete, &wr_id, MGBASE_NULLPTR, &result_lptr)
        )) {
            return false;
        }
        
        // Assign a value to the buffer.
        // TODO: strange naming; buf_lptr instead?
        *result_lptr = params.value;
        
        write_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_atomic_write_params(params, result_lptr, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_write(ibv_params);
        wr.set_write(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        // TODO: Atomicity of ordinary write
        const bool ret = ep_->try_post_send(params.dest_proc, wr);
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_compare_and_swap_async(const rma::compare_and_swap_params<rma::atomic_default_t>& params) const
    {
        mgbase::uint64_t wr_id;
        mgcom::rma::local_ptr<rma::atomic_default_t> result_lptr;
        
        if (MGBASE_UNLIKELY(
            !completer_->try_complete_with_result(params.on_complete, &wr_id, params.result_ptr, &result_lptr)
        )) {
            return false;
        }
        
        compare_and_swap_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_compare_and_swap_params(params, result_lptr, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_compare_and_swap(ibv_params);
        wr.set_compare_and_swap(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        const bool ret = ep_->try_post_send(params.target_proc, wr);
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer_->failed(wr_id);
            return false;
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_fetch_and_add_async(const rma::fetch_and_add_params<rma::atomic_default_t>& params) const
    {
        mgbase::uint64_t wr_id;
        mgcom::rma::local_ptr<rma::atomic_default_t> result_lptr;
        
        if (MGBASE_UNLIKELY(
            !completer_->try_complete_with_result(params.on_complete, &wr_id, params.result_ptr, &result_lptr)
        )) {
            return false;
        }
        
        fetch_and_add_params ibv_params;
        ibv_params.wr_id = wr_id;
        set_fetch_and_add_params(params, result_lptr, &ibv_params);
        
        scatter_gather_entry sge{};
        send_work_request wr{};
        
        sge.set_fetch_and_add(ibv_params);
        wr.set_fetch_and_add(ibv_params, sge);
        wr.next = MGBASE_NULLPTR;
        
        const bool ret = ep_->try_post_send(params.target_proc, wr);
        
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

