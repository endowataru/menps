
#include "common/rma/rma.hpp"
#include "rma.hpp"
#include "atomic.hpp"
#include "contiguous.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

class mpi_requester
    : public emulated_contiguous_requester
    , public emulated_atomic_requester
{
public:
    mpi_requester(rpc::requester& req, mpi_interface& mi)
        : emulated_contiguous_requester(req, mi)
        , emulated_atomic_requester(req)
        { }
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_rma_requester(rpc::requester& req, mpi_interface& mi)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<rma::requester>{ new mpi_requester(req, mi) };
}

} // namespace mpi
} // namespace mgcom

