
#include "rpc_receiver.hpp"
#include "rpc_receiver.impl.hpp"
#include "device/fjmpi/remote_notice.hpp"

namespace mgcom {

namespace rpc {

namespace /*unnamed*/ {

rpc_receiver g_receiver;

} // unnamed namespace

void initialize_receiver(rpc_connection_pool& pool)
{
    g_receiver.initialize(pool);
}

void finalize_receiver()
{
    g_receiver.finalize();
}

namespace untyped {

void register_handler(const handler_id_t id, const handler_function_t callback)
{
    g_receiver.register_handler(id, callback);
}

} // namespace untyped

} // namespace rpc

namespace fjmpi {

void remote_notice(const int nic, const int pid, const int /*tag*/)
{
    rpc::g_receiver.notify(
        static_cast<process_id_t>(pid)
    ,   nic
    );
}

} // namespace fjmpi

} // namespace mgcom

