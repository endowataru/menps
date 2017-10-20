
#include "endpoint.hpp"
#include "device/fjmpi/fjmpi.hpp"
#include "device/fjmpi/fjmpi_error.hpp"
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {

fjmpi_endpoint::fjmpi_endpoint(int* const argc, char*** const argv)
    : mpi::mpi_endpoint{argc, argv}
{
    fjmpi_error::assert_zero(
        FJMPI_Rdma_init()
    );
    
    MEFDN_LOG_DEBUG("msg:Initialized FJMPI.");
}

fjmpi_endpoint::~fjmpi_endpoint()
{
    fjmpi_error::assert_zero(
        FJMPI_Rdma_finalize()
    );
    
    MEFDN_LOG_DEBUG("msg:Finalized FJMPI.");
}

mefdn::unique_ptr<fjmpi_endpoint> make_endpoint(int* argc, char*** argv)
{
    return mefdn::make_unique<fjmpi_endpoint>(argc, argv);
}

} // namespace fjmpi
} // namespace mecom
} // namespace menps

