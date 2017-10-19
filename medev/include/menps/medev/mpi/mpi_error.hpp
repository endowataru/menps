
#pragma once

#include <menps/medev/mpi/mpi.hpp>
#include <stdexcept>
#include <string>

namespace menps {
namespace medev {
namespace mpi {

std::string get_error_string(int err_code) noexcept;

int get_error_class(int err_code) noexcept;

std::string get_error_description(int err_code) noexcept;

std::string get_comm_name(MPI_Comm) noexcept;

class mpi_error
    : public std::runtime_error
{
public:
    explicit mpi_error(const std::string& what_arg) noexcept
        : std::runtime_error(what_arg) { }
    
    static void check(const int err_code)
    {
        if (err_code != MPI_SUCCESS)
            throw mpi_error(get_error_description(err_code));
    }
};

} // namespace mpi
} // namespace medev
} // namespace menps

