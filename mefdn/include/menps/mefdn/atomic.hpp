
#pragma once

#include <atomic>

namespace menps {
namespace mefdn {

using std::atomic;

using std::memory_order_relaxed;
using std::memory_order_consume;
using std::memory_order_acquire;
using std::memory_order_release;
using std::memory_order_acq_rel;
using std::memory_order_seq_cst;

} // namespace mefdn
} // namespace menps

