
#pragma once

#include <mpi.h>

namespace mgcom {

struct mpi3_error
{
    static void check(int err) {
        if (err != MPI_SUCCESS)
            throw mpi3_error();
    }
};

} // namespace mgcom

