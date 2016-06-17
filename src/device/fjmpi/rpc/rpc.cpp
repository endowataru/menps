
#include "rpc.hpp"
#include "rpc_receiver.hpp"
#include "rpc_sender.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

void initialize();
void finalize();

namespace /*unnamed*/ {

class fjmpi_requester
    : public requester
{
public:
    fjmpi_requester()
    {
        fjmpi::rpc::initialize();
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

mgbase::unique_ptr<requester> make_requester()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new fjmpi_requester);
}

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

