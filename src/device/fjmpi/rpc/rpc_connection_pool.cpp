
#include "rpc_connection_pool.hpp"
#include "rpc_receiver.hpp"
#include "rpc_sender.hpp"

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

rpc_connection_pool g_pool;

} // unnamed namespace

void initialize()
{
    g_pool.initialize();
    
    initialize_receiver(g_pool);
    
    initialize_sender(g_pool);
}

void finalize()
{
    finalize_sender();
    
    finalize_receiver();
    
    g_pool.finalize();
}

} // namespace rpc
} // namespace mgcom

