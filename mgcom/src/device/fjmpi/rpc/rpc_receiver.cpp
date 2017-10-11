
#include "rpc_receiver.hpp"
#include "rpc_receiver.impl.hpp"
#include "device/fjmpi/remote_notice.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

mgbase::unique_ptr<rpc_receiver> g_receiver;

} // unnamed namespace

void initialize_receiver(rpc_connection_pool& pool)
{
    g_receiver.reset(new rpc_receiver(pool));
}

void finalize_receiver()
{
    g_receiver.reset();
}

void register_handler_to_receiver(const untyped::register_handler_params& params)
{
    g_receiver->register_handler(params);
}

} // namespace rpc

void remote_notice(const int nic, const int pid, const int /*tag*/)
{
    rpc::g_receiver->notify(
        static_cast<process_id_t>(pid)
    ,   nic
    );
}

} // namespace fjmpi
} // namespace mgcom

