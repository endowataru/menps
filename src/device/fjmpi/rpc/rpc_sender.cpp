
#include "rpc_sender.hpp"
#include "rpc_sender.impl.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

rpc_sender g_sender;

} // unnamed namespace

void initialize_sender(fjmpi_interface& fi, rpc_connection_pool& pool)
{
    g_sender.initialize(fi, pool);
}

void finalize_sender()
{
    // do nothing
}

MGBASE_WARN_UNUSED_RESULT
bool try_call_async(const untyped::call_params& params)
{
    return g_sender.try_call(
        params.proc
    ,   params.handler_id
    ,   params.arg_ptr
    ,   params.arg_size
    ,   params.return_ptr
    ,   params.return_size
    ,   params.on_complete
    );
}

namespace untyped {

bool try_send_reply(
    const int               client_rank
,   const message_buffer&   request_buf
,   void* const             reply_data
,   const int               reply_size
) {
    return g_sender.try_send_reply(
        client_rank
    ,   request_buf
    ,   reply_data
    ,   reply_size
    );
}

} // namespace untyped

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

