
#include <mgdev/ibv/attributes.hpp>
#include <mgdev/ibv/ibv_error.hpp>

namespace mgdev {
namespace ibv {

device_attr_t query_device(ibv_context* const dev)
{
    device_attr_t attr = device_attr_t();
    
    #ifdef MGDEV_IBV_EXP_SUPPORTED
    const auto ret_exp = ibv_exp_query_device(dev, &attr);
    if (ret_exp == 0)
        return attr;
    
    // fall back to non-exp
    #endif
    
    const auto ret = ibv_query_device(dev,
        reinterpret_cast<ibv_device_attr*>(&attr));
    
    if (ret != 0)
        throw ibv_error("ibv_query_device() failed", ret);
    
    return attr;
}

port_attr_t query_port(ibv_context* const dev, const port_num_t port_num)
{
    port_attr_t attr = port_attr_t();
    
    #ifdef MGDEV_IBV_EXP_SUPPORTED
    const auto ret_exp = ibv_exp_query_port(dev, port_num, &attr);
    if (ret_exp == 0)
        return attr;
    
    // fall back to non-exp
    #endif
    
    const auto ret = ibv_query_port(dev, port_num, 
        reinterpret_cast<ibv_port_attr*>(&attr));
    
    if (ret != 0)
        throw ibv_error("ibv_query_port() failed", ret);
    
    return attr;
}

bool is_only_masked_atomics(const device_attr_t& dev_attr) MGBASE_NOEXCEPT
{
    return (dev_attr.exp_atomic_cap & IBV_EXP_ATOMIC_HCA_REPLY_BE) &&
        (dev_attr.exp_device_cap_flags & IBV_EXP_DEVICE_EXT_ATOMICS);
}

} // namespace ibv
} // namespace mgdev

