
#pragma once

#include "device/mpi/mpi.hpp"

namespace menps {
namespace mecom {
namespace mpi {

class mpi_completer_base
{
public:
    virtual ~mpi_completer_base() = default;
    
    mpi_completer_base(const mpi_completer_base&) = delete;
    mpi_completer_base& operator = (const mpi_completer_base&) = delete;
    
    virtual bool full() const noexcept = 0;
    
    struct complete_params
    {
        const MPI_Request&                  req;
        MPI_Status*                         status;
        const mefdn::callback<void ()>&    on_complete;
    };
    
    virtual void complete(const complete_params&) = 0;

protected:
    mpi_completer_base() noexcept = default;
};

} // namespace mpi
} // namespace mecom
} // namespace menps

