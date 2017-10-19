
#pragma once

#include <menps/meult/sm/impl/sm_scheduler.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/memory/shared_ptr.hpp>

namespace menps {
namespace meult {
namespace sm {

typedef mefdn::shared_ptr<sm_scheduler>    scheduler_ptr;

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
    MEFDN_ASSERT(ptr != nullptr);
    return *ptr;
}

} // namespace sm
} // namespace meult
} // namespace menps

