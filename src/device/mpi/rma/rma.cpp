
#include "common/rma/rma.hpp"
#include "rma.hpp"
#include "atomic.hpp"
#include "contiguous.hpp"

#include <mgbase/logging/logger.hpp>

#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

namespace /*unnamed*/ {

class mpi_requester
    : public requester
{
public:
    explicit mpi_requester(mpi_interface& mi)
    {
        initialize_contiguous(mi);
        initialize_atomic();
    }
    
    virtual ~mpi_requester()
    {
        finalize_atomic();
        finalize_contiguous();
    }
    
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_read_async(params);
    }
    
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_write_async(params);
    }
    
    virtual bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_atomic_read_async(params);
    }
    
    virtual bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_atomic_write_async(params);
    }
    
    virtual bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_compare_and_swap_async(params);
    }
    
    virtual bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) MGBASE_OVERRIDE {
        return mgcom::mpi::rma::try_fetch_and_add_async(params);
    }
};

class mpi_registrator
    : public registrator
{
public:
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE {
        // do nothing
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region (but doing nothing.)\tptr:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(params.local_ptr)
        ,   params.size_in_bytes
        );
        
        return untyped::make_local_region(untyped::make_region_key(params.local_ptr, 0 /*unused*/), 0);
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        MGBASE_LOG_DEBUG(
            "msg:Use remote region. (but doing nothing.)"
        );
        
        return untyped::make_remote_region(params.key, 0 /* unused */);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& /*params*/) MGBASE_OVERRIDE {
        // do nothing
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester(mpi_interface& mi)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<requester>(new mpi_requester(mi));
}

mgbase::unique_ptr<registrator> make_registrator()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<registrator>(new mpi_registrator);
}

} // namespace rma
} // namespace mpi
} // namespace mgcom

