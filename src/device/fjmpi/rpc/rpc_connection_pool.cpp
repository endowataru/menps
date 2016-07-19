
#include "rpc_connection_pool.hpp"
#include "rpc_receiver.hpp"
#include "rpc_sender.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

rpc_connection_pool g_pool;

} // unnamed namespace

void initialize(fjmpi_interface& fi, mpi::mpi_interface& mi, rma::allocator& alloc, rma::registrator& reg)
{
    g_pool.initialize(mi, alloc, reg);
    
    initialize_receiver(g_pool);
    
    initialize_sender(fi, g_pool);
}

void finalize()
{
    finalize_sender();
    
    finalize_receiver();
    
    g_pool.finalize();
}

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

