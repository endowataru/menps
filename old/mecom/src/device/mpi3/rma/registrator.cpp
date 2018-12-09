
#include "rma.hpp"
#include "common/rma/rma.hpp"
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {
namespace rma {
namespace /*unnamed*/ {

class mpi3_registrator
    : public registrator
{
public:
    /*implicit*/ mpi3_registrator(mpi3::mpi3_interface& mi, const MPI_Win win)
        : mi_(mi)
        , win_(win)
    { }
    
    MEFDN_NODISCARD
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MEFDN_OVERRIDE
    {
        const MPI_Aint addr = mi_.attach({
            params.local_ptr
        ,   static_cast<MPI_Aint>(params.size_in_bytes)
        ,   this->win_
        });
        
        MEFDN_LOG_DEBUG(
            "msg:Registered region\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mefdn::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return untyped::make_local_region(untyped::make_region_key(reinterpret_cast<void*>(addr), 0), 0);
    }
    
    MEFDN_NODISCARD
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MEFDN_OVERRIDE
    {
        MEFDN_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params) MEFDN_OVERRIDE
    {
        mi_.detach({
            untyped::to_raw_pointer(params.region)
        ,   this->win_
        });
    }
    
private:
    mpi3::mpi3_interface&   mi_;
    const MPI_Win           win_;
};

} // unnamed namespace
} // namespace rma

namespace mpi3 {

mefdn::unique_ptr<rma::registrator> make_rma_registrator(mpi3_interface& mi, const MPI_Win win)
{
    return mefdn::make_unique<rma::mpi3_registrator>(mi, win);
}

} // namespace mpi3

} // namespace mecom
} // namespace menps

