
#pragma once

#include <mgdev/common.hpp>
#include <ucs/type/status.h>
#include <string>
#include <stdexcept>

namespace mgdev {
namespace ucx {

class ucx_error
    : public std::runtime_error
{
public:
    /*implicit*/ ucx_error(const std::string& what_arg) MGBASE_NOEXCEPT
        : std::runtime_error(what_arg)
    { }
    
    /*implicit*/ ucx_error(const std::string& what_arg, ucs_status_t status);
};

} // namespace ucx
} // namespace mgdev

