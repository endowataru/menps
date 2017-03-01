
#include <mgdev/ibv/device_context.hpp>
#include <mgdev/ibv/attributes.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>

namespace mgdev {
namespace ibv {

device_context open_device(ibv_device* const dev)
{
    const auto ctx = ibv_open_device(dev);
    if (ctx == MGBASE_NULLPTR) {
        throw ibv_error("ibv_open_device() failed");
    }
    
    return device_context(ctx);
}

void device_context_deleter::operator () (ibv_context* const ctx) const MGBASE_NOEXCEPT
{
    if (ctx == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_close_device(ctx); // ignore error
}

ibv_device_attr device_context::query_device() const {
    return ibv::query_device(this->get());
}
ibv_port_attr device_context::query_port(const port_num_t port_num) const {
    return ibv::query_port(this->get(), port_num);
}
node_id_t device_context::get_node_id(const port_num_t port_num) const {
    return { this->query_port(port_num).lid };
}

} // namespace ibv
} // namespace mgdev

