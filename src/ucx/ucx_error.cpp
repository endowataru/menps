
#include <mgdev/ucx/ucx_error.hpp>
#include <mgbase/external/fmt.hpp>

namespace mgdev {
namespace ucx {

ucx_error::ucx_error(const std::string& what_arg, const ucs_status_t status)
    : std::runtime_error(fmt::format("{} (status:{})", what_arg, status))
{ }

} // namespace ucx
} // namespace mgdev

