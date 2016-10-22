
#pragma once

#include <mgult/root_scheduler.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgult {
namespace sm {

mgbase::unique_ptr<root_scheduler> make_scheduler();

} // namespace sm
} // namespace mgult

