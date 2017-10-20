
#pragma once

#include "mpi.hpp"
#include <stdexcept>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace mecom {

class mpi_error
    : public std::runtime_error
{
public:
    explicit mpi_error(const std::string& what_arg) noexcept
        : std::runtime_error(what_arg) { }
    
    static std::string get_description(const int err_code)
    {
        const int err_class = get_error_class(err_code);
        const std::string code_str = get_error_string(err_code);
        const std::string class_str = get_error_string(err_class);
        
        return fmt::format(
            "{}:{}, {}:{}"
        ,   err_class
        ,   class_str
        ,   err_code
        ,   code_str
        );
    }
    
    
    static std::string get_error_string(const int err_code) noexcept
    {
        char result[MPI_MAX_ERROR_STRING];
        int len;
        
        MPI_Error_string(err_code, result, &len); // Ignore error
        
        return result;
    }
    
    static int get_error_class(const int err_code)
    {
        int err_class;
        
        MPI_Error_class(err_code, &err_class); // Ignore error
        
        return err_class;
    }
    
    static void check(const int err_code) {
        if (err_code != MPI_SUCCESS)
            throw mpi_error(get_description(err_code));
    }
};

} // namespace mecom
} // namespace menps

