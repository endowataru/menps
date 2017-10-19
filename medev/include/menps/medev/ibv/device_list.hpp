
#pragma once

#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ibv {

struct device_list_deleter
{
    void operator () (ibv_device**) const noexcept;
};

class device_list
    : public mefdn::unique_ptr<ibv_device* [], device_list_deleter>
{
    typedef mefdn::unique_ptr<ibv_device* [], device_list_deleter>    base;
    
public:
    device_list() noexcept = default;
    
    explicit device_list(ibv_device** p, mefdn::size_t size)
        : base(p)
        , size_(size)
    { }
    
    ~device_list() /*noexcept*/ = default;
    
    device_list(const device_list&) = delete;
    device_list& operator = (const device_list&) = delete;
    
    device_list(device_list&&) noexcept = default;
    device_list& operator = (device_list&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return size_;
    }
    
    ibv_device* get_by_index(mefdn::size_t idx) const;
    
    ibv_device* get_by_name(const char* name) const;
    
private:
    mefdn::size_t size_;
};

device_list get_device_list();

} // namespace ibv
} // namespace medev
} // namespace menps

