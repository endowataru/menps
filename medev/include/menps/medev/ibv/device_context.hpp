
#pragma once

#include <menps/medev/ibv/attributes.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ibv {

struct device_context_deleter
{
    void operator () (ibv_context*) const noexcept;
};

class device_context
    : public mefdn::unique_ptr<ibv_context, device_context_deleter>
{
    typedef mefdn::unique_ptr<ibv_context, device_context_deleter> base;
    
public:
    device_context() noexcept = default;
    
    explicit device_context(ibv_context* const ctx)
        : base(ctx)
    { }
    
    ~device_context() /*noexcept*/ = default;
    
    device_context(const device_context&) = delete;
    device_context& operator = (const device_context&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(device_context, base)
    
    device_attr_t query_device() const;
    
    port_attr_t query_port(port_num_t port_num) const;
    
    node_id_t get_node_id(port_num_t port_num) const;
};

device_context open_device(ibv_device*);

} // namespace ibv
} // namespace medev
} // namespace menps

