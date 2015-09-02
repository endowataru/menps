
#include "mpi_base.hpp"
#include "rma.hpp"
#include "am.hpp"
#include <mpi.h>

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize();
    am::initialize();
}

void finalize()
{
    MPI_Barrier(MPI_COMM_WORLD);
    
    am::finalize();
    rma::finalize();
    
    mpi_base::finalize();
}

}

