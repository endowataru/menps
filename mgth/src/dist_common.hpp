
#pragma once

#include <mgth/common.hpp>
#include <mgctx.hpp>
//#include <mgult/fcontext.hpp>
#include <mgult/ult_id.hpp>

namespace mgth {

class dist_worker;

using mgult::ult_id;

typedef mgbase::size_t      worker_rank_t;

//typedef mgult::fcontext<dist_worker, dist_worker>  context_t;
typedef mgctx::context<dist_worker*>  context_t;

} // namespace mgth

