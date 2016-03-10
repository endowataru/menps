
#pragma once

#include "context.hpp"

namespace mgcom {
namespace ibv {

struct device_attributes
    : public ibv_device_attr
{
    // trivially copyable
    
    void query(context& ctx)
    {
        const int ret = ibv_query_device(&ctx.get(), this);
        if (ret != 0)
            throw ibv_error("ibv_query_device() failed", ret);
    }
};

struct port_attributes
    : public ibv_port_attr
{
    // trivially copyable
    
    void query(context& ctx, const mgbase::uint8_t port_num)
    {
        const int ret = ibv_query_port(&ctx.get(), port_num, this);
        if (ret != 0)
            throw ibv_error("ibv_query_port() failed", ret);
    }
};

} // namespace ibv
} // namespace mgcom

