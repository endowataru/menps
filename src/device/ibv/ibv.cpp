
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "device/mpi1/command/mpi1_command_queue.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "common/rma/region_allocator.hpp"
#include "device/mpi/collective/collective.hpp"
#include "ibv.hpp"
#include <mgcom/collective.hpp>

#include <mgbase/logging/logger.hpp>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mgcom::mpi::initialize(argc, argv);
    
    mgcom::mpi1::initialize_command_queue();
    
    rpc::initialize();
    
    collective::initialize();
    
    ibv::initialize();
    
    mgcom::mpi::native_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    mgcom::mpi::native_barrier();
    
    ibv::finalize();
    
    collective::finalize();
    
    rpc::finalize();
    
    mgcom::mpi1::finalize_command_queue();
    
    mgcom::mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

} // namespace mgcom

