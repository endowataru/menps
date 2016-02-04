
#pragma once

#include "device/mpi/mpi_error.hpp"

namespace mgcom {

struct mpi3_error
    : public mpi_error
{
    static void check(int err) {
        if (err != MPI_SUCCESS)
            emit();
    }
    
    MGBASE_NORETURN static void emit()
    {
        throw mpi3_error();
    }
};

} // namespace mgcom

