
#include "rma.impl.hpp"
#include "device/mpi/rma/atomic.hpp"

namespace mgcom {
namespace fjmpi {
namespace rma {

namespace /*unnamed*/ {

rma_impl g_impl;

class fjmpi_requester
    : public requester
{
public:
    fjmpi_requester()
    {
        mgcom::mpi::rma::initialize_atomic();
    }
    
    virtual ~fjmpi_requester()
    {
        mgcom::mpi::rma::finalize_atomic();
    }
    
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE {
        return mgcom::fjmpi::rma::untyped::try_read_async(params);
    }
    
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE {
        return mgcom::fjmpi::rma::untyped::try_write_async(params);
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


class fjmpi_registrator
    : public registrator
{
public:
    fjmpi_registrator()
    {
        g_impl.initialize();
    }
    
    virtual ~fjmpi_registrator()
    {
        g_impl.finalize();
    }
    
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MGBASE_OVERRIDE
    {
        mgbase::uint64_t laddr;
        
        const int memid = g_impl.register_memory(
            params.local_ptr
        ,   params.size_in_bytes
        ,   &laddr
        );
        
        return untyped::make_local_region(
            untyped::make_region_key(
                params.local_ptr
            ,   static_cast<mgbase::uint64_t>(memid)
            )
        ,   laddr
        );
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MGBASE_OVERRIDE
    {
        const mgbase::uint64_t raddr
            = g_impl.get_remote_addr(
                static_cast<int>(params.proc_id)
            ,   static_cast<int>(params.key.info)
            );
        
        return untyped::make_remote_region(params.key, raddr);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params)
    {
        g_impl.deregister_memory(
            static_cast<int>(params.region.key.info)
        );
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    return mgbase::unique_ptr<requester>(new fjmpi_requester);
}

mgbase::unique_ptr<registrator> make_registrator()
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<registrator>(new fjmpi_registrator);
}

} // namespace rma
} // namespace fjmpi
} // namespace mgcom

