
#pragma once

#include "device/mpi/mpi_interface.hpp"

namespace menps {
namespace mecom {
namespace mpi {

class mpi_communicator
{
public:
    explicit mpi_communicator(mpi_interface& mi, MPI_Comm comm);
    
    mpi_communicator(mpi_interface& mi, MPI_Comm comm, const char* name);
    
    mpi_communicator(const mpi_communicator&) = delete;
    mpi_communicator& operator = (const mpi_communicator&) = delete;
    
    ~mpi_communicator();
    
    MPI_Comm get() const noexcept {
        return comm_;
    }
    
private:
    mpi_interface& mi_;
    MPI_Comm comm_;
};

} // namespace mpi
} // namespace mecom
} // namespace menps

