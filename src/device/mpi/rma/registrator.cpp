
#include "rma.hpp"
#include "common/rma/region_allocator.hpp"
#include "common/rma/rma.hpp"
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

class mpi_registrator
    : public rma::registrator
{
public:
    virtual rma::untyped::local_region register_region(const rma::untyped::register_region_params& params) MGBASE_OVERRIDE {
        // do nothing
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region (but doing nothing.)\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return rma::untyped::make_local_region(rma::untyped::make_region_key(params.local_ptr, 0 /*unused*/), 0);
    }
    
    virtual rma::untyped::remote_region use_remote_region(const rma::untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        MGBASE_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return rma::untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const rma::untyped::deregister_region_params& /*params*/) MGBASE_OVERRIDE {
        // do nothing
    }
};

} // unnamed namespace

mgbase::unique_ptr<rma::registrator> make_rma_registrator()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<rma::registrator>(new mpi_registrator);
}

} // namespace mpi
} // namespace mgcom

