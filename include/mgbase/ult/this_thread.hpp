
#pragma once

#include <mgbase/threading/this_thread.hpp>

namespace mgbase {
namespace ult {
namespace this_thread {

using mgbase::this_thread::get_id;

#ifdef MGBASE_ULT_DISABLE_YIELD
inline void yield() MGBASE_NOEXCEPT { }
#else
using mgbase::this_thread::yield;
#endif


} // namespace this_thread
} // namespace ult
} // namespace mgbase

