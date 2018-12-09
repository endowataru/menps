
#pragma once

#include <menps/mefdn/lang.hpp>
#include <string>
#include <stdexcept>

namespace menps {
namespace medev {
namespace ibv {

class ibv_error
    : public std::runtime_error
{
public:
    /*implicit*/ ibv_error(const std::string& what_arg) noexcept
        : std::runtime_error(what_arg)
    { }
    
    /*implicit*/ ibv_error(const std::string& what_arg, int err_code);
};

} // namespace ibv
} // namespace medev
} // namespace menps

