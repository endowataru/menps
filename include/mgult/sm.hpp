
#pragma once

#include <mgult/scheduler.hpp>
#include <mgbase/unique_ptr.hpp>
#include <mgult/sm/thread.hpp>

namespace mgult {

typedef mgbase::unique_ptr<scheduler>   scheduler_ptr;

scheduler_ptr make_sm_scheduler();

} // namespace mgult

