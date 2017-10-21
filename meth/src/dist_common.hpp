
#pragma once

#include <menps/meth/common.hpp>
#include <menps/mectx.hpp>
//#include <menps/meult/fcontext.hpp>
#include <menps/meult/ult_id.hpp>

namespace menps {
namespace meth {

class dist_worker;

using meult::ult_id;

typedef mefdn::size_t      worker_rank_t;

//typedef meult::fcontext<dist_worker, dist_worker>  context_t;
typedef mectx::context<dist_worker*>  context_t;

} // namespace meth
} // namespace menps

