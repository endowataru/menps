
#include "device/mpi/mpi_base.hpp"
#include "rma/rma.hpp"
#include "device/mpi/am/am.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize();
    am::initialize();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    am::finalize();
    rma::finalize();
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

