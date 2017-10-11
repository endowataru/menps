
#include "common/rma/rma.hpp"
#include "rma.hpp"
#include <mgcom/rpc/rpc_policy.hpp>
#include "atomic.impl.hpp"
#include "contiguous.impl.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

class mpi_requester;

struct mpi_requster_policy
    : rpc::rpc_policy
{
    typedef mpi_requester   derived_type;
    
    typedef rma::requester  requester_interface_type;
    
    typedef rma::atomic_default_t                                   atomic_value_type;
    typedef rma::async_atomic_read_params<atomic_value_type>        async_atomic_read_params_type;
    typedef rma::async_atomic_write_params<atomic_value_type>       async_atomic_write_params_type;
    typedef rma::async_compare_and_swap_params<atomic_value_type>   async_compare_and_swap_params_type;
    typedef rma::async_fetch_and_add_params<atomic_value_type>      async_fetch_and_add_params_type;
};

class mpi_requester
    : public emulated_contiguous<mpi_requster_policy>
    , public emulated_atomic<mpi_requster_policy>
{
public:
    explicit mpi_requester(rpc::requester& rqstr, mpi_interface& mi)
        : rqstr_(rqstr)
        , mi_(mi)
    {
        emulated_contiguous<mpi_requster_policy>::setup();
        emulated_atomic<mpi_requster_policy>::setup();
    }
        
private:
    friend class emulated_contiguous<mpi_requster_policy>;
    friend class emulated_atomic<mpi_requster_policy>;
    
    rpc::requester& get_rpc_requester() const MGBASE_NOEXCEPT {
        return rqstr_;
    }
    mpi_interface& get_mpi_interface() const MGBASE_NOEXCEPT {
        return mi_;
    }
    
    rpc::requester& rqstr_;
    mpi_interface& mi_;
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_rma_requester(rpc::requester& req, mpi_interface& mi)
{
    return mgbase::make_unique<mpi_requester>(req, mi);
}

} // namespace mpi
} // namespace mgcom

