
#pragma once

#include <menps/meth/common.hpp>
#include <menps/meult/root_scheduler.hpp>
#include <menps/medsm/space_ref.hpp>
#include <menps/mefdn/memory/allocatable.hpp>

namespace menps {
namespace meth {

using meult::scheduler;
using meult::root_scheduler;

using meult::loop_func_type;

typedef mefdn::unique_ptr<root_scheduler>  dist_scheduler_ptr;

struct dist_scheduler_config
{
    medsm::space_ref& space;
    void* stack_segment_ptr;
};

dist_scheduler_ptr make_dist_scheduler(const dist_scheduler_config&);

} // namespace meth
} // namespace menps

