
#pragma once

#include "mpi.hpp"

namespace mgcom {

struct mpi_error
{
    MGBASE_NORETURN static void emit()
    {
        throw mpi_error();
    }
    
    static void check(int err) {
        if (err != MPI_SUCCESS)
            emit();
    }
};

} // namespace mgcom

