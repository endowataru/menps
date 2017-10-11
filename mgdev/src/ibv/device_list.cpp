
#include <mgdev/ibv/device_list.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/external/fmt.hpp>

namespace mgdev {
namespace ibv {

device_list get_device_list()
{
    int num_devices = 0;
    const auto p = ibv_get_device_list(&num_devices);
    if (p == MGBASE_NULLPTR) {
        throw ibv_error("ibv_get_device_list() failed");
    }
    
    return device_list(p, static_cast<mgbase::size_t>(num_devices));
}

void device_list_deleter::operator () (ibv_device** const p) const MGBASE_NOEXCEPT
{
    if (p == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_free_device_list(p); // ignore error
}

ibv_device* device_list::get_by_index(const mgbase::size_t idx) const
{
    MGBASE_ASSERT(*this);
    MGBASE_ASSERT(idx < this->size());
    
    return (*this)[idx];
}

ibv_device* device_list::get_by_name(const char* const name) const
{
    MGBASE_ASSERT(*this);
    
    for (int i = 0; (*this)[i]; ++i)
    {
        const auto dev = (*this)[i];
        
        const char* const dev_name = ibv_get_device_name(dev);
        
        if (std::strcmp(dev_name, name) == 0)
        {
            return dev;
        }
    }
    
    throw ibv_error(fmt::format("device \"{}\" was not found", name));
}

} // namespace ibv
} // namespace mgdev

