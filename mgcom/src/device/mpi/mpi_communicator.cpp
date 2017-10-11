
#include "device/mpi/mpi_communicator.hpp"

namespace mgcom {
namespace mpi {

mpi_communicator::mpi_communicator(mpi_interface& mi, MPI_Comm comm)
    : mi_(mi)
    , comm_(mi.comm_dup(comm)) { }

mpi_communicator::mpi_communicator(mpi_interface& mi, MPI_Comm comm, const char* const name)
    : mi_(mi)
    , comm_(mi.comm_dup(comm, name)) { }

mpi_communicator::~mpi_communicator()
{
    mi_.comm_free(&comm_);
}

} // namespace mpi
} // namespace mgcom


