
#pragma once

#include <menps/mecom/rma/local_ptr.hpp>
#include <menps/mecom/rma/allocation.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename T>
struct default_local_delete
{
    void operator() (const mecom::rma::local_ptr<T>& ptr)
    {
        mecom::rma::deallocate(ptr);
    }
};

template <typename T, typename TD = default_local_delete<T>>
class unique_local_ptr;

namespace detail {

template <typename T, typename TD>
struct unique_local_ptr_traits
{
    typedef unique_local_ptr<T, TD>     derived_type;
    typedef mecom::rma::local_ptr<T>    resource_type;
    typedef TD                          deleter_type;
};

} // namespace detail

template <typename T, typename TD>
class unique_local_ptr
    : public mefdn::basic_unique_resource<detail::unique_local_ptr_traits<T, TD>>
{
    typedef detail::unique_local_ptr_traits<T, TD>  policy;
    typedef mefdn::basic_unique_resource<policy>   base;
    typedef typename policy::resource_type          resource_type;
    
public:
    unique_local_ptr() /*noexcept (TODO)*/ = default;
    
    explicit unique_local_ptr(resource_type ptr) noexcept
        : base(mefdn::move(ptr))
    { }
    
    // move-only
    
    unique_local_ptr(const unique_local_ptr&) = delete;
    unique_local_ptr& operator = (const unique_local_ptr&) = delete;
    
    unique_local_ptr(unique_local_ptr&&) /*noexcept (TODO) */ = default;
    unique_local_ptr& operator = (unique_local_ptr&&) /*noexcept (TODO)*/ = default;
    
private:
    friend class mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept
    {
        return !!this->get_resource();
    }
    
    void set_unowned()
    {
        this->get_resource() = local_ptr<T>{};
    }
    
    void set_owned()
    {
        // do nothing
    }
};

} // namespace rma
} // namespace mecom
} // namespace menps

