
#include "rma.hpp"
#include "common/rma/rma.hpp"
#include <mgbase/logger.hpp>

namespace mgcom {
namespace rma {
namespace /*unnamed*/ {

class mpi3_registrator
    : public registrator
{
public:
    explicit mpi3_registrator(mpi3::mpi3_interface& mi)
        : mi_(mi) { }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE
    {
        const MPI_Aint addr = mi_.attach({
            params.local_ptr
        ,   static_cast<MPI_Aint>(params.size_in_bytes)
        });
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return untyped::make_local_region(untyped::make_region_key(reinterpret_cast<void*>(addr), 0), 0);
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        MGBASE_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params) MGBASE_OVERRIDE
    {
        mi_.detach({ untyped::to_raw_pointer(params.region) });
    }
    
private:
    mpi3::mpi3_interface& mi_;
};

} // unnamed namespace
} // namespace rma

namespace mpi3 {

mgbase::unique_ptr<rma::registrator> make_rma_registrator(mpi3_interface& mi)
{
    return mgbase::make_unique<rma::mpi3_registrator>(mi);
}

} // namespace mpi3

} // namespace mgcom

