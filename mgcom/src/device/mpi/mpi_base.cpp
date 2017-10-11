
#include "mpi_base.hpp"

namespace mgcom {
namespace mpi {

std::string get_comm_name(const MPI_Comm comm)
{
    char name[MPI_MAX_OBJECT_NAME];
    int len;
    
    mpi_error::check(
        MPI_Comm_get_name(comm, name, &len)
    );
    
    return name;
}

} // namespace mpi
} // namespace mgcom

