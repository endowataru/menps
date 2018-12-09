
#include <menps/medev/ibv/device_context.hpp>
#include <menps/medev/ibv/attributes.hpp>
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medev {
namespace ibv {

device_context open_device(ibv_device* const dev)
{
    const auto ctx = ibv_open_device(dev);
    if (ctx == nullptr) {
        throw ibv_error("ibv_open_device() failed");
    }
    
    return device_context(ctx);
}

void device_context_deleter::operator () (ibv_context* const ctx) const noexcept
{
    if (ctx == nullptr) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_close_device(ctx); // ignore error
}

device_attr_t device_context::query_device() const {
    return ibv::query_device(this->get());
}
port_attr_t device_context::query_port(const port_num_t port_num) const {
    return ibv::query_port(this->get(), port_num);
}
node_id_t device_context::get_node_id(const port_num_t port_num) const {
    return { this->query_port(port_num).lid };
}

} // namespace ibv
} // namespace medev
} // namespace menps

