
#pragma once

#include <menps/meult/fcontext.hpp>
#include <menps/mectx.hpp>

namespace menps {
namespace meult {
namespace sm {

class sm_worker;

typedef mectx::context<sm_worker*>  context_t;

} // namespace sm
} // namespace meult
} // namespace menps

