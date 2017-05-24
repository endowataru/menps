
#pragma once

#include <mgth/common.hpp>
#include <mgult/root_scheduler.hpp>
#include <mgdsm/space_ref.hpp>
#include <mgbase/memory/allocatable.hpp>

namespace mgth {

using mgult::scheduler;
using mgult::root_scheduler;

using mgult::loop_func_type;

typedef mgbase::unique_ptr<root_scheduler>  dist_scheduler_ptr;

struct dist_scheduler_config
{
    mgdsm::space_ref& space;
    void* stack_segment_ptr;
};

dist_scheduler_ptr make_dist_scheduler(const dist_scheduler_config&);

} // namespace mgth

