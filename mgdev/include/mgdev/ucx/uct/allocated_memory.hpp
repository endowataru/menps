
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/scope/basic_unique_resource.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct allocated_memory_deleter
{
    void operator() (uct_allocated_memory_t&&) const MGBASE_NOEXCEPT;
};

class allocated_memory;

namespace detail {

struct allocated_memory_policy
{
    typedef allocated_memory            derived_type;
    typedef uct_allocated_memory_t      resource_type;
    typedef allocated_memory_deleter    deleter_type;
};

} // namespace detail

class allocated_memory
    : public mgbase::basic_unique_resource<detail::allocated_memory_policy>
{
    typedef mgbase::basic_unique_resource<detail::allocated_memory_policy>  base;
    
public:
    allocated_memory() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit allocated_memory(uct_allocated_memory_t mem)
        : base(mgbase::move(mem))
    { }
    
    allocated_memory(const allocated_memory&) = delete;
    allocated_memory& operator = (const allocated_memory&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(allocated_memory, base)
    
private:
    friend class mgbase::basic_unique_resource_access;
    
    bool is_owned() const MGBASE_NOEXCEPT {
        return this->get_resource().address != MGBASE_NULLPTR;
    }
    
    void set_unowned() MGBASE_NOEXCEPT {
        this->get_resource() = uct_allocated_memory_t();
    }
    void set_owned() MGBASE_NOEXCEPT {
        // do nothing
    }
};

allocated_memory allocate_memory(uct_iface_t* iface, size_t length, unsigned int flags);

} // namespace uct
} // namesapce ucx
} // namespace mgdev

