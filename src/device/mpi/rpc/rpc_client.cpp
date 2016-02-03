
#include "rpc_client.impl.hpp"

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

rpc_client g_client;

} // unnamed namespace

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
    return g_client.try_call(
        proc
    ,   handler_id
    ,   arg_ptr
    ,   arg_size
    ,   return_ptr
    ,   return_size
    ,   on_complete
    );
}

} // namespace untyped

} // namespace rpc
} // namespace mgcom

