
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

// Define as specified in C++17 (or Parallelism TS).

namespace execution {

struct sequenced_policy { };

struct parallel_policy { };

struct parallel_unsequenced_policy { };

} // namespace execution

} // namespace mefdn
} // namespace menps

