
#include "rma.hpp"
#include "rma.impl.hpp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace mpi3 {
namespace rma {

namespace /*unnamed*/ {

mpi3_rma g_impl;

} // unnamed namespace

MPI_Win get_win() MGBASE_NOEXCEPT
{
    return g_impl.get_win();
}

namespace /*unnamed*/ {

class mpi3_registrator
    : public registrator
{
public:
    mpi3_registrator()
    {
        g_impl.initialize();
    }
    
    virtual ~mpi3_registrator()
    {
        g_impl.finalize();
    }
    
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE
    {
        const MPI_Aint addr = g_impl.attach(params.local_ptr, static_cast<MPI_Aint>(params.size_in_bytes));
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return untyped::make_local_region(untyped::make_region_key(reinterpret_cast<void*>(addr), 0), 0);
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        MGBASE_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params)
    {
        g_impl.detach(untyped::to_raw_pointer(params.region));
    }
};

} // unnamed namespace

mgbase::unique_ptr<registrator> make_registrator()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<registrator>(new mpi3_registrator);
}

} // namespace rma
} // namespace mpi3
} // namespace mgcom

