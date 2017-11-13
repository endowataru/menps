
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct allocated_memory_deleter
{
    void operator() (uct_allocated_memory_t&&) const noexcept;
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
    : public mefdn::basic_unique_resource<detail::allocated_memory_policy>
{
    typedef mefdn::basic_unique_resource<detail::allocated_memory_policy>  base;
    
public:
    allocated_memory() noexcept = default;
    
    explicit allocated_memory(uct_allocated_memory_t mem)
        : base(mefdn::move(mem))
    { }
    
    allocated_memory(const allocated_memory&) = delete;
    allocated_memory& operator = (const allocated_memory&) = delete;
    
    allocated_memory(allocated_memory&&) /*noexcept (TODO)*/ = default;
    allocated_memory& operator = (allocated_memory&&) /*noexcept (TODO)*/ = default;
    
private:
    friend class mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept {
        return this->get_resource().address != nullptr;
    }
    
    void set_unowned() noexcept {
        this->get_resource() = uct_allocated_memory_t();
    }
    void set_owned() noexcept {
        // do nothing
    }
};

allocated_memory allocate_memory(uct_iface_t* iface, size_t length, unsigned int flags, const char* name);

} // namespace uct
} // namesapce ucx
} // namespace medev
} // namespace menps

