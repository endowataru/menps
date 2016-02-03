
#include "device/mpi/mpi_base.hpp"
#include "rma/rma.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi3/command/mpi3_command_queue.hpp"
#include <mgcom/collective.hpp>

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi::initialize(argc, argv);
    
    mgcom::mpi3::initialize_command_queue();
    
    rma::initialize();
    
    rpc::initialize();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    rpc::finalize();
    
    rma::finalize();
    
    mgcom::mpi3::finalize_command_queue();
    
    mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

