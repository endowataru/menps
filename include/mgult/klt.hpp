
#pragma once

#include <mgbase/thread.hpp>
#include <mgbase/mutex.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/sync_flag.hpp>

namespace mgult {
namespace klt {

using mgbase::mutex;
using mgbase::condition_variable;

using mgbase::lock_guard;
using mgbase::unique_lock;

using mgbase::thread;

namespace this_thread = mgbase::this_thread;

using mgbase::spinlock;

#ifdef MGBASE_ULT_DISABLE_YIELD
inline void yield() MGBASE_NOEXCEPT { }
#else
using mgbase::this_thread::yield;
#endif

using mgbase::sync_flag;

} // namespace klt
} // namespace mgult

