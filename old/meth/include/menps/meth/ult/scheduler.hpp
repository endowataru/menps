
#pragma once

#include <menps/meth/common.hpp>
#include <menps/meult/scheduler.hpp>
#include <menps/mefdn/memory/shared_ptr.hpp>

namespace menps {
namespace meth {
namespace ult {

using meult::scheduler;

typedef mefdn::shared_ptr<scheduler>   scheduler_ptr;

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
} // namespace meth
} // namespace menps

