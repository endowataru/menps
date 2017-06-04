
#pragma once

#include <mgcom/common.hpp>

#ifdef MGCOM_FORK_EXECUTOR_THREAD
    #include <mgult/offload/basic_fork_offload_queue.hpp>
#else
    #include <mgult/offload/basic_cv_offload_queue.hpp>
#endif

namespace mgcom {

template <typename Policy>
class basic_locked_command_queue
#ifdef MGCOM_FORK_EXECUTOR_THREAD
    : public mgult::basic_fork_offload_queue<Policy>
#else
    : public mgult::basic_cv_offload_queue<Policy>
#endif
{ };

} // namespace mgcom

