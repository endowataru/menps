
#include "rpc_sender.hpp"
#include "rpc_sender.impl.hpp"

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

rpc_sender g_sender;

} // unnamed namespace

void initialize_sender(rpc_connection_pool& pool)
{
    g_sender.initialize(pool);
}

void finalize_sender()
{
    // do nothing
}

namespace untyped {

bool try_remote_call_async(
    process_id_t                proc
,   handler_id_t                handler_id
,   const void*                 arg_ptr
,   index_t                     arg_size
,   void*                       return_ptr
,   index_t                     return_size
,   const mgbase::operation&    on_complete
) {
    return g_sender.try_call(
        proc
    ,   handler_id
    ,   arg_ptr
    ,   arg_size
    ,   return_ptr
    ,   return_size
    ,   on_complete
    );
}

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
} // namespace mgcom

