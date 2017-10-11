
#pragma once

#include <mgdev/mpi/mpi.hpp>
#include <stdexcept>
#include <string>

namespace mgdev {
namespace mpi {

std::string get_error_string(int err_code) MGBASE_NOEXCEPT;

int get_error_class(int err_code) MGBASE_NOEXCEPT;

std::string get_error_description(int err_code) MGBASE_NOEXCEPT;

std::string get_comm_name(MPI_Comm) MGBASE_NOEXCEPT;

class mpi_error
    : public std::runtime_error
{
public:
    explicit mpi_error(const std::string& what_arg) MGBASE_NOEXCEPT
        : std::runtime_error(what_arg) { }
    
    static void check(const int err_code)
    {
        if (err_code != MPI_SUCCESS)
            throw mpi_error(get_error_description(err_code));
    }
};

} // namespace mpi
} // namespace mgdev

