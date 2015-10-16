
#include "common/mpi_base.hpp"
#include "rma/rma.hpp"
#include "am/am.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {

namespace /*unnamed*/ {

std::string get_state() {
    static index_t number = 0;
    fmt::MemoryWriter w;
    w.write("proc:{}\tlog_id:{}\t", current_process_id(), number++);
    return w.str();
}

}

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize();
    am::initialize();
    
    mgbase::logger::set_state_callback(get_state);
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    MPI_Barrier(MPI_COMM_WORLD);
    
    am::finalize();
    rma::finalize();
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

}

