
#include "registrator.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "address.hpp"
#include <mgdev/ibv/memory_region.hpp>

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
        const auto pd = ep_.get_pd();
        
        auto mr = ibv::make_memory_region(pd, params.local_ptr, params.size_in_bytes);
        
        // Explicitly release the ownership.
        const auto mr_ptr = mr.release();
        
        const untyped::region_key key = { params.local_ptr, mr_ptr->lkey };
        const untyped::local_region reg = { key, reinterpret_cast<mgbase::uint64_t>(mr_ptr) };
        return reg;
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        untyped::remote_region region;
        region.key = params.key;
        return region;
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params) MGBASE_OVERRIDE
    {
        const auto mr_ptr = ibv::to_mr(params.region);
        
        // Wrap the ownership again.
        ibv::memory_region mr(mr_ptr);
        
        // Deregister the MR by the destructor.
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

