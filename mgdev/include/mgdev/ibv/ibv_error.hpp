
#pragma once

#include <mgbase/lang.hpp>
#include <string>
#include <stdexcept>

namespace mgdev {
namespace ibv {

class ibv_error
    : public std::runtime_error
{
public:
    /*implicit*/ ibv_error(const std::string& what_arg) MGBASE_NOEXCEPT
        : std::runtime_error(what_arg)
    { }
    
    /*implicit*/ ibv_error(const std::string& what_arg, int err_code);
};

} // namespace ibv
} // namespace mgdev

