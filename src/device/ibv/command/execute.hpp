
#pragma once

#include "copy_params.hpp"
#include "device/ibv/native/send_work_request.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"

namespace mgcom {
namespace ibv {

template <typename Completer, typename QueuePair>
MGBASE_WARN_UNUSED_RESULT
inline bool try_execute_read(const read_params& params, Completer& completer, QueuePair& qp)
{
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(
        !completer.try_complete(&wr_id)
    )) {
        return false;
    }
    
    completer.set_on_complete(wr_id, params.on_complete);
    
    scatter_gather_entry sge{};
    send_work_request wr{};
    
    copy_params_to(params, &wr, &sge);
    wr.wr_id = wr_id;
    wr.next = MGBASE_NULLPTR;
    
    const bool ret = qp.try_post_send(wr);
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        completer->failed(wr_id);
        return false;
    }
}

template <typename Completer, typename QueuePair>
MGBASE_WARN_UNUSED_RESULT
inline bool try_execute_write(const write_params& params, Completer& completer, QueuePair& qp)
{
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(
        !completer.try_complete(&wr_id)
    )) {
        return false;
    }
    
    completer_.set_on_complete(wr_id, params.on_complete);
    
    scatter_gather_entry sge{};
    send_work_request wr{};
    
    copy_params_to(params, &wr, &sge);
    wr.wr_id = wr_id;
    wr.next = MGBASE_NULLPTR;
    
    const bool ret = qp.try_post_send(wr);
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        completer->failed(wr_id);
        return false;
    }
}



} // namespace ibv
} // namespace mgcom

