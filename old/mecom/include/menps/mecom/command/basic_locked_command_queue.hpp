
#pragma once

#include <menps/mecom/common.hpp>

#ifdef MECOM_FORK_EXECUTOR_THREAD
    #include <menps/meult/offload/basic_fork_offload_queue.hpp>
#else
    #include <menps/meult/offload/basic_cv_offload_queue.hpp>
#endif

namespace menps {
namespace mecom {

template <typename Policy>
class basic_locked_command_queue
#ifdef MECOM_FORK_EXECUTOR_THREAD
    : public meult::basic_fork_offload_queue<Policy>
#else
    : public meult::basic_cv_offload_queue<Policy>
#endif
{ };

} // namespace mecom
} // namespace menps

