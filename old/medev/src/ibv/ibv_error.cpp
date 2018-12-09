
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <string>

namespace menps {
namespace medev {
namespace ibv {

ibv_error::ibv_error(const std::string& what_arg, const int err_code)
    : std::runtime_error(
        fmt::format("{} (code:{})", what_arg, err_code)
    )
{ }

} // namespace ibv
} // namespace medev
} // namespace menps


