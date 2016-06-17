
#include "rpc_client.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

rpc_client g_client;

} // unnamed namespace

bool try_call_async(const untyped::call_params& params)
{
    return g_client.try_call(
        params.proc
    ,   params.handler_id
    ,   params.arg_ptr
    ,   params.arg_size
    ,   params.return_ptr
    ,   params.return_size
    ,   params.on_complete
    );
}

} // namespace rpc
} // namespace mpi
} // namespace mgcom

