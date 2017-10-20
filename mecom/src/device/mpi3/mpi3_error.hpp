
#pragma once

#include "device/mpi/mpi_error.hpp"

namespace menps {
namespace mecom {

class mpi3_error
    : public mpi_error
{
public:
    explicit mpi3_error(const std::string& what_arg) noexcept
        : mpi_error(what_arg) { }
    
    
    static void check(const int err_code) {
        if (err_code != MPI_SUCCESS)
            throw mpi3_error(mpi_error::get_description(err_code));
    }
};

} // namespace mecom
} // namespace menps

