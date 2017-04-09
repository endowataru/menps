
#include <mgdev/ibv/attributes.hpp>
#include <mgdev/ibv/ibv_error.hpp>

namespace mgdev {
namespace ibv {

#ifdef MGDEV_IBV_EXP_SUPPORTED
    #define QUERY_DEVICE    ibv_exp_query_device
    #define QUERY_PORT      ibv_exp_query_port
#else
    #define QUERY_DEVICE    ibv_query_device
    #define QUERY_PORT      ibv_query_port
#endif

device_attr_t query_device(ibv_context* const dev)
{
    device_attr_t attr = device_attr_t();
    
    const auto ret = QUERY_DEVICE(dev, &attr);
    if (ret != 0)
        throw ibv_error("ibv_query_device() failed", ret);
    
    return attr;
}

port_attr_t query_port(ibv_context* const dev, const port_num_t port_num)
{
    port_attr_t attr = port_attr_t();
    
    const auto ret = QUERY_PORT(dev, port_num, &attr);
    if (ret != 0)
        throw ibv_error("ibv_query_port() failed", ret);
    
    return attr;
}

bool is_only_masked_atomics(const device_attr_t& dev_attr) MGBASE_NOEXCEPT
{
    return (dev_attr.exp_atomic_cap & IBV_EXP_ATOMIC_HCA_REPLY_BE) &&
        (dev_attr.exp_device_cap_flags & IBV_EXP_DEVICE_EXT_ATOMICS);
}

#undef QUERY_DEVICE
#undef QUERY_PORT

} // namespace ibv
} // namespace mgdev

