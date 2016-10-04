
#pragma once

#include <mgult/scheduler.hpp>
#include <mgdsm/dsm_interface.hpp>

namespace mgth {

using mgult::scheduler;

using mgult::loop_func_t;

typedef mgbase::unique_ptr<scheduler>   dist_scheduler_ptr;

dist_scheduler_ptr make_dist_scheduler(mgdsm::dsm_interface&);

} // namespace mgth

