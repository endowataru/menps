
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/external/fmt.hpp>
#include <string>

namespace mgdev {
namespace ibv {

ibv_error::ibv_error(const std::string& what_arg, const int err_code)
    : std::runtime_error(
        fmt::format("{} (code:{})", what_arg, err_code)
    )
{ }

} // namespace ibv
} // namespace mgdev


