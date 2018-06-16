
#pragma once

#include <menps/medev2/ucx/ucp/config.hpp>
#include <menps/medev2/ucx/ucp/context.hpp>
#include <menps/medev2/ucx/ucp/memory.hpp>
#include <menps/medev2/ucx/ucp/packed_rkey.hpp>
#include <menps/medev2/ucx/ucp/remote_key.hpp>
#include <menps/medev2/ucx/ucp/worker.hpp>
#include <menps/medev2/ucx/ucp/worker_address.hpp>
#include <menps/medev2/ucx/ucp/endpoint.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct ucp_policy
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    using config_type = config<P>;
    using context_type = context<P>;
    using memory_type = memory<P>;
    using packed_rkey_type = packed_rkey<P>;
    using remote_key_type = remote_key<P>;
    using worker_type = worker<P>;
    using worker_address_type = worker_address<P>;
    using endpoint_type = endpoint<P>;
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

