
#pragma once

#include "verbs.hpp"
#include <cstring>
#include <mgbase/external/fmt.hpp>

namespace mgcom {
namespace ibv {

class device_list
    : mgbase::noncopyable
{
public:
    device_list()
        : dev_list_(MGBASE_NULLPTR) { }
    
    ~device_list()
    {
        if (dev_list_ != MGBASE_NULLPTR)
            free_list();
    }
    
    void get_list()
    {
        MGBASE_ASSERT(dev_list_ == MGBASE_NULLPTR);
        
        dev_list_ = ibv_get_device_list(&num_devices_);
        if (dev_list_ == MGBASE_NULLPTR || num_devices_ < 0)
            throw ibv_error("ibv_get_device_list() failed");
    }
    
    void free_list() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(dev_list_ != MGBASE_NULLPTR);
        
        ibv_free_device_list(dev_list_); // ignore error
        
        dev_list_ = MGBASE_NULLPTR;
    }
    
    ibv_device& get_by_index(index_t idx)
    {
        MGBASE_ASSERT(dev_list_ != MGBASE_NULLPTR);
        return *dev_list_[idx];
    }
    
    ibv_device& get_by_name(const char* const name)
    {
        MGBASE_ASSERT(dev_list_ != MGBASE_NULLPTR);
        
        for (int i = 0; i < num_devices_; ++i)
        {
            const char* const dev_name = ibv_get_device_name(dev_list_[i]);
            
            if (std::strcmp(dev_name, name) == 0)
            {
                return *dev_list_[i];
            }
        }
        
        throw ibv_error(fmt::format("device \"{}\" was not found", name));
    }
    
private:
    ibv_device**    dev_list_;
    int             num_devices_;
};

} // namespace ibv
} // namespace mgcom

