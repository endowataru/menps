
#pragma once

#include <menps/meult/scheduler.hpp>
#include <menps/meult/sm/thread.hpp>
#include <menps/meult/sm/initializer.hpp>
#include <menps/meult/sm/scoped_task.hpp>

namespace menps {
namespace meult {
namespace sm {

namespace this_thread {

inline void yield()
{
    get_scheduler().yield();
}

} // namespace this_thread

} // namespace sm
} // namespace meult
} // namespace menps

