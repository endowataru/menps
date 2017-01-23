
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/external/fmt.hpp>
#include <string>
#include <stdexcept>

namespace mgdev {
namespace ibv {

class ibv_error
    : public std::runtime_error
{
public:
    explicit ibv_error(const std::string& what_arg) MGBASE_NOEXCEPT
        : std::runtime_error(what_arg) { }
    
    ibv_error(const std::string& what_arg, const int err_code)
        : std::runtime_error(
            fmt::format("{} (code:{})", what_arg, err_code)
        ) { }
};

} // namespace ibv
} // namespace mgdev

