
#pragma once

#include <mgult/scheduler.hpp>
#include <mgult/sm/thread.hpp>
#include <mgult/sm/initializer.hpp>
#include <mgult/sm/scoped_task.hpp>

namespace mgult {
namespace sm {

namespace this_thread {

inline void yield()
{
    get_scheduler().yield();
}

} // namespace this_thread

} // namespace sm
} // namespace mgult

