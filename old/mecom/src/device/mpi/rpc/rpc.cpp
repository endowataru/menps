
#include "rpc_server.hpp"
#include "rpc_client.hpp"
#include <menps/medev/mpi/communicator.hpp>

namespace menps {
namespace mecom {
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
        MEFDN_LOG_DEBUG("msg:Finalize RPC.");
    }
};

mefdn::unique_ptr<rpc::requester> make_rpc_requester(mpi_interface& mi, endpoint& ep)
{
    return mefdn::make_unique<rpc_requester>(mi, ep);
}

} // namespace mpi
} // namespace mecom
} // namespace menps
