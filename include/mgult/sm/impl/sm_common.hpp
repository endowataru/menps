
#pragma once

#include <mgult/fcontext.hpp>
#include <mgctx.hpp>

namespace mgult {
namespace sm {

class sm_worker;

typedef mgctx::context<sm_worker*>  context_t;

} // namespace sm
} // namespace mgult

