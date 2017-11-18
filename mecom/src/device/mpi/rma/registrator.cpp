
#include "rma.hpp"
#include "common/rma/rma.hpp"
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {
namespace mpi {

namespace /*unnamed*/ {

class mpi_registrator
    : public rma::registrator
{
public:
    virtual rma::untyped::local_region register_region(const rma::untyped::register_region_params& params) MEFDN_OVERRIDE {
        // do nothing
        
        MEFDN_LOG_DEBUG(
            "msg:Registered region (but doing nothing.)\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mefdn::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return rma::untyped::make_local_region(rma::untyped::make_region_key(params.local_ptr, 0 /*unused*/), 0);
    }
    
    virtual rma::untyped::remote_region use_remote_region(const rma::untyped::use_remote_region_params& params) MEFDN_OVERRIDE
    {
        MEFDN_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return rma::untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const rma::untyped::deregister_region_params& /*params*/) MEFDN_OVERRIDE {
        // do nothing
    }
};

} // unnamed namespace

mefdn::unique_ptr<rma::registrator> make_rma_registrator()
{
    return mefdn::make_unique<mpi_registrator>();
}

} // namespace mpi
} // namespace mecom
} // namespace menps

