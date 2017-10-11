
#pragma once

#include "device/mpi/mpi_error.hpp"

namespace mgcom {

class mpi3_error
    : public mpi_error
{
public:
    explicit mpi3_error(const std::string& what_arg) MGBASE_NOEXCEPT
        : mpi_error(what_arg) { }
    
    
    static void check(const int err_code) {
        if (err_code != MPI_SUCCESS)
            throw mpi3_error(mpi_error::get_description(err_code));
    }
};

} // namespace mgcom

