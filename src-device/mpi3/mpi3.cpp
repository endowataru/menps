
#include "mpi_base.hpp"
#include "rma.hpp"
#include "am.hpp"
#include <mpi.h>

#include <mgbase/logging/logger.hpp>

namespace mgcom {

namespace {

std::string get_state() {
    fmt::MemoryWriter w;
    w.write("[proc={}] ", current_process_id());
    return w.str();
}

}

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize();
    am::initialize();
    
    mgbase::logger::set_state_callback(get_state);
    
    MGBASE_LOG_DEBUG("Initialized.");
}

void finalize()
{
    MPI_Barrier(MPI_COMM_WORLD);
    
    am::finalize();
    rma::finalize();
    
    mpi_base::finalize();
    
    MGBASE_LOG_DEBUG("Finalized.");
}

}

