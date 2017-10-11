
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ibv {

struct device_list_deleter
{
    void operator () (ibv_device**) const MGBASE_NOEXCEPT;
};

class device_list
    : public mgbase::unique_ptr<ibv_device* [], device_list_deleter>
{
    typedef mgbase::unique_ptr<ibv_device* [], device_list_deleter>    base;
    
public:
    device_list() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit device_list(ibv_device** p, mgbase::size_t size)
        : base(p)
        , size_(size)
    { }
    
    ~device_list() /*noexcept*/ = default;
    
    device_list(const device_list&) = delete;
    device_list& operator = (const device_list&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(device_list, base, size_)
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    ibv_device* get_by_index(mgbase::size_t idx) const;
    
    ibv_device* get_by_name(const char* name) const;
    
private:
    mgbase::size_t size_;
};

device_list get_device_list();

} // namespace ibv
} // namespace mgdev

