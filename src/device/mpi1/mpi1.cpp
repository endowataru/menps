
#include "common/mpi_base.hpp"
#include "device/mpi3/rma/rma.hpp"
#include "device/mpi/am/am.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

namespace collective {

void initialize();

} // namespace collective

namespace /*unnamed*/ {

std::string get_state() {
    static index_t number = 0;
    fmt::MemoryWriter w;
    w.write("proc:{}\tlog_id:{}\t", current_process_id(), number++);
    return w.str();
}

} // unnamed namespace

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    
    mgbase::logger::set_state_callback(get_state);
    
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

