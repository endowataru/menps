
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

namespace mgcom {

struct mpi_error
{
    static void check(int err) {
        if (err != MPI_SUCCESS)
            throw mpi_error();
    }
};

}

