
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ibv {

struct device_context_deleter
{
    void operator () (ibv_context*) const MGBASE_NOEXCEPT;
};

class device_context
    : public mgbase::unique_ptr<ibv_context, device_context_deleter>
{
    typedef mgbase::unique_ptr<ibv_context, device_context_deleter> base;
    
public:
    device_context() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit device_context(ibv_context* const ctx)
        : base(ctx)
    { }
    
    ~device_context() /*noexcept*/ = default;
    
    device_context(const device_context&) = delete;
    device_context& operator = (const device_context&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(device_context, base)
    
    ibv_device_attr query_device() const;
    
    ibv_port_attr query_port(port_num_t port_num) const;
    
    node_id_t get_node_id(port_num_t port_num) const;
};

device_context open_device(ibv_device*);

} // namespace ibv
} // namespace mgdev

