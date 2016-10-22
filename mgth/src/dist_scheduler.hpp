
#pragma once

#include <mgult/root_scheduler.hpp>
#include <mgdsm/dsm_interface.hpp>

namespace mgth {

using mgult::scheduler;
using mgult::root_scheduler;

using mgult::loop_func_type;

typedef mgbase::unique_ptr<root_scheduler>  dist_scheduler_ptr;

dist_scheduler_ptr make_dist_scheduler(mgdsm::dsm_interface&);

} // namespace mgth

