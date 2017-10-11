
#pragma once

#include <mgdev/ibv/verbs.hpp>

namespace mgdev {
namespace ibv {

#ifdef MGDEV_IBV_EXP_SUPPORTED
    typedef ibv_exp_device_attr device_attr_t;
    typedef ibv_exp_port_attr   port_attr_t;
#else
    typedef ibv_device_attr     device_attr_t;
    typedef ibv_port_attr       port_attr_t;
#endif

device_attr_t query_device(ibv_context*);

port_attr_t query_port(ibv_context*, port_num_t);

bool is_only_masked_atomics(const device_attr_t&) MGBASE_NOEXCEPT;

} // namespace ibv
} // namespace mgdev

