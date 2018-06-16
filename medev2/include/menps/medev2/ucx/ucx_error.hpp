
#pragma once

#include <menps/medev2/common.hpp>
#include <ucs/type/status.h>
#include <string>
#include <stdexcept>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace medev2 {
namespace ucx {

class ucx_error
    : public std::runtime_error
{
public:
    /*implicit*/ ucx_error(const std::string& what_arg) noexcept
        : std::runtime_error(what_arg)
    { }
    
    /*implicit*/ ucx_error(const std::string& what_arg, ucs_status_t status)
        : std::runtime_error(fmt::format("{} (status:{})", what_arg, status))
    { }
};

} // namespace ucx
} // namespace medev2
} // namespace menps

