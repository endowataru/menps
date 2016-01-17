
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/am/am.hpp"
#include "device/mpi/rma/rma.hpp"
#include "device/mpi/collective/collective.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    
    am::initialize();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    rma::initialize();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    collective::initialize();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    rma::finalize();
    
    am::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

