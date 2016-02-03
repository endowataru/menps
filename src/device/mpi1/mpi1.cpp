
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "device/mpi/command/mpi_command_queue.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/rma/rma.hpp"
#include "device/mpi/collective/collective.hpp"
#include <mgcom/collective.hpp>

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mgcom::mpi::initialize(argc, argv);
    
    mgcom::mpi::initialize_command_queue();
    
    rpc::initialize();
    
    rma::initialize();
    
    collective::initialize();
    
    mgcom::mpi::blocking_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    mgcom::mpi::blocking_barrier();
    
    collective::finalize();
    
    rma::finalize();
    
    rpc::finalize();
    
    mgcom::mpi::finalize_command_queue();
    
    mgcom::mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

