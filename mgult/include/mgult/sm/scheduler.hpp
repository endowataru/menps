
#pragma once

#include <mgult/sm/impl/sm_scheduler.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgult {
namespace sm {

typedef mgbase::shared_ptr<sm_scheduler>    scheduler_ptr;

namespace detail {

inline scheduler_ptr& get_scheduler_ptr()
{
    static scheduler_ptr ptr;
    return ptr;
}

} // namespace detail

inline void set_scheduler(const scheduler_ptr& p)
{
    auto& ptr = detail::get_scheduler_ptr();
    ptr = p;
}

inline sm_scheduler& get_scheduler()
{
    auto& ptr = detail::get_scheduler_ptr();
    MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
    return *ptr;
}

} // namespace sm
} // namespace mgult

