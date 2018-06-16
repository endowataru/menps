
#pragma once

#include <menps/mecom2/rma/alltoall_buffer.hpp>
#include <menps/mecom2/rma/ucp/ucp_rma.hpp>

namespace menps {
namespace mecom2 {

// TODO: Remove this header.

template <typename Elem>
using ucp_alltoall_buffer =
    alltoall_buffer<alltoall_buffer_policy<ucp_rma, Elem>>;

} // namespace mecom2
} // namespace menps

