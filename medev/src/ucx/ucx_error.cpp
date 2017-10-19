
#include <menps/medev/ucx/ucx_error.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace medev {
namespace ucx {

ucx_error::ucx_error(const std::string& what_arg, const ucs_status_t status)
    : std::runtime_error(fmt::format("{} (status:{})", what_arg, status))
{ }

} // namespace ucx
} // namespace medev
} // namespace menps

