
#include <menps/meult/klt/klt_policy.hpp>
#include <menps/meult/backend/mth/ult_policy.hpp>

namespace menps {
namespace meult {

constexpr mefdn::execution::sequenced_policy klt_policy::execution::seq;
constexpr mefdn::execution::parallel_policy klt_policy::execution::par;

constexpr mefdn::execution::sequenced_policy backend::mth::ult_policy::execution::seq;
constexpr mefdn::execution::parallel_policy backend::mth::ult_policy::execution::par;

} // namespace meult
} // namespace menps
