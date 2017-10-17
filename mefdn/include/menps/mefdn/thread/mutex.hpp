
#pragma once

#include <menps/mefdn/lang.hpp>
#include <mutex>

namespace menps {
namespace mefdn {

using std::mutex;

using std::unique_lock;

using std::adopt_lock_t;
using std::defer_lock_t;
using std::try_to_lock_t;

using std::adopt_lock;
using std::defer_lock;
using std::try_to_lock;

} // namespace mefdn
} // namespace menps

