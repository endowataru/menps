
#include "rpc_server.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

rpc_server g_server;


class mpi_requester
    : public requester
{
public:
    mpi_requester()
    {
        g_server.initialize();
    }
    
    virtual ~mpi_requester()
    {
        g_server.finalize();
    }
    
    virtual void register_handler(const untyped::register_handler_params& params)
    {
        g_server.register_handler(params.id, params.callback);
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_call_async(const untyped::call_params& params)
    {
        return mgcom::mpi::rpc::try_call_async(params);
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new mpi_requester);
}

MPI_Comm get_comm()
{
    return g_server.get_comm();
}

} // namespace rpc
} // namespace mpi
} // namespace mgcom

