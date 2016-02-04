
#include "device/fjmpi/command/fjmpi_command_queue.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "rma/rma.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/collective/collective.hpp"
#include "device/mpi/rma/atomic.hpp"
#include "common/rma/region_allocator.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mgcom::mpi::initialize(argc, argv);
    
    mgcom::fjmpi::initialize_command_queue();
    
    mgcom::rma::initialize_contiguous();
    
    mgcom::rpc::initialize();
    
    mgcom::rma::initialize_atomic();
    
    mgcom::rma::initialize_allocator();
    
    mgcom::collective::initialize();
    
    mpi::blocking_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    mgcom::collective::barrier(); 
    
    mgcom::mpi::blocking_barrier();
    
    mgcom::collective::finalize();
    
    mgcom::rma::finalize_allocator();
    
    mgcom::rma::finalize_atomic();
    
    mgcom::rpc::finalize();
    
    mgcom::rma::finalize_contiguous();
    
    mgcom::fjmpi::finalize_command_queue();
    
    mgcom::mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

