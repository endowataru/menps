
#pragma once

#include <mgult/scheduler.hpp>

namespace mgult {

namespace sm {

typedef mgbase::unique_ptr<scheduler>   scheduler_ptr;

namespace detail {

inline scheduler_ptr& get_global_scheduler_ptr()
{
    static scheduler_ptr ptr;
    return ptr;
}

} // namespace detail

inline void set_global_scheduler(scheduler_ptr s)
{
    auto& sched = detail::get_global_scheduler_ptr();
    sched = mgbase::move(s);
}

inline scheduler& get_global_scheduler()
{
    auto& sched = detail::get_global_scheduler_ptr();
    MGBASE_ASSERT(sched != MGBASE_NULLPTR);
    return *sched;
}

} // namespace sm

} // namespace mgult

