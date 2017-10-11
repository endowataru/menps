
#include "rpc.hpp"
#include "rpc_receiver.hpp"
#include "rpc_sender.hpp"
#include "rpc_connection_pool.hpp"
#include "requester.hpp"

namespace mgcom {
namespace fjmpi {

namespace rpc {

namespace /*unnamed*/ {

class fjmpi_requester
    : public requester
{
public:
    fjmpi_requester(fjmpi_interface& fi, mpi::mpi_interface& mi, rma::allocator& alloc, rma::registrator& reg)
    {
        fjmpi::rpc::initialize(fi, mi, alloc, reg);
    }
    
    virtual ~fjmpi_requester()
    {
        fjmpi::rpc::finalize();
    }
    
    virtual void register_handler(const untyped::register_handler_params& params)
    {
        fjmpi::rpc::register_handler_to_receiver(params);
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_call_async(const untyped::call_params& params)
    {
        return fjmpi::rpc::try_call_async(params);
    }
};

} // unnamed namespace

} // namespace rpc

mgbase::unique_ptr<rpc::requester> make_rpc_requester(fjmpi_interface& fi, mpi::mpi_interface& mi, rma::allocator& alloc, rma::registrator& reg)
{
    // TODO: replace with make_unique
    return mgbase::make_unique<rpc::fjmpi_requester>(fi, mi, alloc, reg);
}

} // namespace fjmpi
} // namespace mgcom

