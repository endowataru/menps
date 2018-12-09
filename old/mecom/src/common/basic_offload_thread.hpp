
#pragma once

#include <menps/mecom/common.hpp>

#ifdef MECOM_FORK_EXECUTOR_THREAD
    #include <menps/meult/offload/basic_fork_offload_thread.hpp>
#else
    #include <menps/meult/offload/basic_cv_offload_thread.hpp>
#endif

namespace menps {
namespace mecom {

template <typename Policy>
class basic_offload_thread
#ifdef MECOM_FORK_EXECUTOR_THREAD
    : public meult::basic_fork_offload_thread<Policy>
#else
    : public meult::basic_cv_offload_thread<Policy>
#endif
{ };

} // namespace mecom
} // namespace menps

