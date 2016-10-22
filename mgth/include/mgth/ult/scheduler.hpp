
#pragma once

#include <mgult/scheduler.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgth {
namespace ult {

using mgult::scheduler;

typedef mgbase::shared_ptr<scheduler>   scheduler_ptr;

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

scheduler& get_scheduler();

} // namespace ult
} // namespace mgth

