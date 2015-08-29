
#include "mpi_base.hpp"
#include "rma.hpp"
#include "am.hpp"

namespace mgcom {

void initialize(int* argc, char*** argv)
{
    mpi_base::initialize(argc, argv);
    rma::initialize();
    am::initialize();
}

void finalize()
{
    am::finalize();
    rma::finalize();
    mpi_base::finalize();
}

}

