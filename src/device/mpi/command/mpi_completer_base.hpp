
#pragma once

#include "device/mpi/mpi.hpp"

namespace mgcom {
namespace mpi {

class mpi_completer_base
{
public:
    virtual ~mpi_completer_base() MGBASE_EMPTY_DEFINITION
    
    mpi_completer_base(const mpi_completer_base&) = delete;
    mpi_completer_base& operator = (const mpi_completer_base&) = delete;
    
    virtual bool full() const MGBASE_NOEXCEPT = 0;
    
    struct complete_params
    {
        const MPI_Request&                  req;
        MPI_Status*                         status;
        const mgbase::callback<void ()>&    on_complete;
    };
    
    virtual void complete(const complete_params&) = 0;

protected:
    mpi_completer_base() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
};

} // namespace mpi
} // namespace mgcom

