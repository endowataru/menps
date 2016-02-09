
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
    
    mgcom::rma::initialize_contiguous();
    
    mgcom::fjmpi::initialize_command_queue();
    
    mgcom::rma::initialize_allocator();
    
    mgcom::rpc::initialize();
    
    mgcom::rma::initialize_atomic();
    
    mgcom::collective::initialize();
    
    mpi::native_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    mgcom::collective::barrier(); 
    
    mgcom::mpi::native_barrier();
    
    mgcom::collective::finalize();
    
    mgcom::rma::finalize_atomic();
    
    mgcom::rpc::finalize();
    
    mgcom::rma::finalize_allocator();
    
    mgcom::fjmpi::finalize_command_queue();
    
    mgcom::rma::finalize_contiguous();
    
    mgcom::mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

