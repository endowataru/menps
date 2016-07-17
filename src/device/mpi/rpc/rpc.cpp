
#include "rpc_server.impl.hpp"
#include "rpc_client.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class mpi_requester
    : public requester
{
public:
    explicit mpi_requester(mpi_interface& mi)
        : server_(mi)
        , client_(mi, server_.get_comm()) { }
    
    virtual ~mpi_requester() MGBASE_EMPTY_DEFINITION
    
    virtual void register_handler(const untyped::register_handler_params& params)
    {
        server_.register_handler(params);
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_call_async(const untyped::call_params& params)
    {
        return client_.try_call_async(params);
    }

private:
    rpc_server server_;
    rpc_client client_;
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester(mpi_interface& mi)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new mpi_requester(mi));
}

} // namespace rpc
} // namespace mpi
} // namespace mgcom
