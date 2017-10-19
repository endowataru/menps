
#pragma once

#include <menps/medev/ibv/verbs.hpp>

namespace menps {
namespace medev {
namespace ibv {

#ifdef MEDEV_IBV_EXP_SUPPORTED
    typedef ibv_exp_device_attr device_attr_t;
    typedef ibv_exp_port_attr   port_attr_t;
#else
    typedef ibv_device_attr     device_attr_t;
    typedef ibv_port_attr       port_attr_t;
#endif

device_attr_t query_device(ibv_context*);

port_attr_t query_port(ibv_context*, port_num_t);

bool is_only_masked_atomics(const device_attr_t&) noexcept;

} // namespace ibv
} // namespace medev
} // namespace menps

