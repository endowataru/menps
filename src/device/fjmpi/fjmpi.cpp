
#include "device/mpi/mpi_base.hpp"
#include "rma/rma.hpp"
#include "am/am.hpp"
#include "device/mpi/collective/collective.hpp"
#include "device/mpi/rma/atomic.hpp"
#include "common/rma/region_allocator.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize_contiguous();
    am::initialize();
    rma::initialize_atomic();
    rma::initialize_allocator();
    collective::initialize();
    
    mpi_base::native_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier(); 
    mpi_base::native_barrier();
    
    collective::finalize();
    rma::finalize_allocator();
    rma::finalize_atomic();
    am::finalize();
    rma::finalize_contiguous();
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

