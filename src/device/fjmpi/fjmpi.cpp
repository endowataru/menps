
#include "device/mpi/mpi_base.hpp"
#include "rma/rma.hpp"
#include "am/am.hpp"
#include "device/mpi/collective/collective.hpp"
#include "device/mpi/rma/atomic.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize_contiguous();
    am::initialize();
    rma::initialize_atomic();
    collective::initialize();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier(); 
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    collective::finalize();
    rma::finalize_atomic();
    am::finalize();
    rma::finalize_contiguous();
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

