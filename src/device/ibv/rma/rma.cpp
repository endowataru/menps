
#include "rma.hpp"

namespace mgcom {
namespace ibv {
namespace rma {

namespace /*unnamed*/ {

class ibv_requester
    : public requester
{
public:
    ibv_requester()
    {
        
    }
    
    virtual ~ibv_requester()
    {
    }
    
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::untyped::try_read_async(params);
    }
    
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::untyped::try_write_async(params);
    }
    
    virtual bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::try_atomic_read_async(params);
    }
    
    virtual bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::try_atomic_write_async(params);
    }
    
    virtual bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::try_compare_and_swap_async(params);
    }
    
    virtual bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::try_fetch_and_add_async(params);
    }
};

class ibv_registrator
    : public registrator
{
public:
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::untyped::register_region(params);
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE {
        return mgcom::ibv::rma::untyped::use_remote_region(params);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params) {
        return mgcom::ibv::rma::untyped::deregister_region(params);
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new ibv_requester);
}

mgbase::unique_ptr<registrator> make_registrator()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<registrator>(new ibv_registrator);
}

} // namespace rma
} // namespace ibv
} // namespace mgcom

