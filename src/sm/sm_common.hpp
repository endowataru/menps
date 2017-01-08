
#pragma once

#include <mgult/fcontext.hpp>
#include <mgctx.hpp>

namespace mgult {

class sm_worker;

typedef mgctx::context<sm_worker*>  context_t;

//typedef fcontext<sm_worker, sm_worker>  context_t;

} // namespace mgult

