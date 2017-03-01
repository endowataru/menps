
#include <mgdev/ibv/attributes.hpp>
#include <mgdev/ibv/ibv_error.hpp>

namespace mgdev {
namespace ibv {

ibv_device_attr query_device(ibv_context* const dev)
{
    ibv_device_attr attr = ibv_device_attr();
    
    const auto ret = ibv_query_device(dev, &attr);
    if (ret != 0)
        throw ibv_error("ibv_query_device() failed", ret);
    
    return attr;
}

ibv_port_attr query_port(ibv_context* const dev, const port_num_t port_num)
{
    ibv_port_attr attr = ibv_port_attr();
    
    const auto ret = ibv_query_port(dev, port_num, &attr);
    if (ret != 0)
        throw ibv_error("ibv_query_port() failed", ret);
    
    return attr;
}

} // namespace ibv
} // namespace mgdev

