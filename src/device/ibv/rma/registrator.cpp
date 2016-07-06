
#include "registrator.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "address.hpp"

namespace mgcom {

namespace rma {
namespace /*unnamed*/ {

class ibv_registrator
    : public registrator
{
public:
    ibv_registrator(ibv::endpoint& ep)
        : ep_(ep) { }
    
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE
    {
        ibv_mr& mr = ep_.register_memory(params.local_ptr, params.size_in_bytes);
        
        const untyped::region_key key = { params.local_ptr, mr.lkey };
        const untyped::local_region reg = { key, reinterpret_cast<mgbase::uint64_t>(&mr) };
        return reg;
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        untyped::remote_region region;
        region.key = params.key;
        return region;
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params)
    {
        ibv_mr* const mr = ibv::to_mr(params.region);
        ep_.deregister_memory(*mr);
    }
    
private:
    ibv::endpoint& ep_;
};

} // unnamed namespace
} // namespace rma

namespace ibv {

mgbase::unique_ptr<rma::registrator> make_rma_registrator(endpoint& ep)
{
    return mgbase::make_unique<rma::ibv_registrator>(ep);
}

} // namespace ibv

} // namespace mgcom

