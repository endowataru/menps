
#include "rpc_server.impl.hpp"

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

rpc_server g_server;

} // unnamed namespace

void initialize()
{
    g_server.initialize();
}

void finalize()
{
    g_server.finalize();
}

MPI_Comm get_comm()
{
    return g_server.get_comm();
}

namespace untyped {

void register_handler(const handler_id_t id, const handler_function_t callback) {
    g_server.register_handler(id, callback);
}

} // namespace untyped

} // namespace rpc
} // namespace mgcom

