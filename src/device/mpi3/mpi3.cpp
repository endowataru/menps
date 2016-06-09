
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi3/rma/rma.hpp"
#include "device/mpi3/command/mpi3_command_queue.hpp"
#include "common/rma/region_allocator.hpp"
#include <mgcom/collective.hpp>

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi::initialize(argc, argv);
    
    mgcom::mpi3::initialize_command_queue();
    
    rma::initialize();
    
    rma::initialize_allocator();
    
    rpc::initialize();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    rpc::finalize();
    
    rma::finalize_allocator();
    
    rma::finalize();
    
    mgcom::mpi3::finalize_command_queue();
    
    mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

