
#include "rpc_server.hpp"
#include "rpc_client.hpp"
#include <mgdev/mpi/communicator.hpp>

namespace mgcom {
namespace mpi {

class rpc_requester
    : public rpc_server
    , public rpc_client
{
public:
    explicit rpc_requester(mpi_interface& mi, endpoint& ep)
        : rpc_base(mi)
        , rpc_server(mi)
        , rpc_client(mi, ep)
    { }
    
    ~rpc_requester()
    {
        MGBASE_LOG_DEBUG("msg:Finalize RPC.");
    }
};

mgbase::unique_ptr<rpc::requester> make_rpc_requester(mpi_interface& mi, endpoint& ep)
{
    return mgbase::make_unique<rpc_requester>(mi, ep);
}

} // namespace mpi
} // namespace mgcom
