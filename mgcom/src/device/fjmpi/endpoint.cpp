
#include "endpoint.hpp"
#include "device/fjmpi/fjmpi.hpp"
#include "device/fjmpi/fjmpi_error.hpp"
#include <mgbase/logger.hpp>

namespace mgcom {
namespace fjmpi {

fjmpi_endpoint::fjmpi_endpoint(int* const argc, char*** const argv)
    : mpi::mpi_endpoint{argc, argv}
{
    fjmpi_error::assert_zero(
        FJMPI_Rdma_init()
    );
    
    MGBASE_LOG_DEBUG("msg:Initialized FJMPI.");
}

fjmpi_endpoint::~fjmpi_endpoint()
{
    fjmpi_error::assert_zero(
        FJMPI_Rdma_finalize()
    );
    
    MGBASE_LOG_DEBUG("msg:Finalized FJMPI.");
}

mgbase::unique_ptr<fjmpi_endpoint> make_endpoint(int* argc, char*** argv)
{
    return mgbase::make_unique<fjmpi_endpoint>(argc, argv);
}

} // namespace fjmpi
} // namespace mgcom

