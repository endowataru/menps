
#pragma once

#include <mgult/fcontext.hpp>

namespace mgult {

class sm_worker;

typedef fcontext<sm_worker, sm_worker>  context_t;

} // namespace mgult

