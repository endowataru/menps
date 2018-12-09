
#include <menps/medev/ibv/device_list.hpp>
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace medev {
namespace ibv {

device_list get_device_list()
{
    int num_devices = 0;
    const auto p = ibv_get_device_list(&num_devices);
    if (p == nullptr) {
        throw ibv_error("ibv_get_device_list() failed");
    }
    
    return device_list(p, static_cast<mefdn::size_t>(num_devices));
}

void device_list_deleter::operator () (ibv_device** const p) const noexcept
{
    if (p == nullptr) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_free_device_list(p); // ignore error
}

ibv_device* device_list::get_by_index(const mefdn::size_t idx) const
{
    MEFDN_ASSERT(*this);
    MEFDN_ASSERT(idx < this->size());
    
    return (*this)[idx];
}

ibv_device* device_list::get_by_name(const char* const name) const
{
    MEFDN_ASSERT(*this);
    
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
} // namespace medev
} // namespace menps

