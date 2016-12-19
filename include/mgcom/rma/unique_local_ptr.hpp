
#pragma once

#include <mgcom/rma/local_ptr.hpp>
#include <mgcom/rma/allocation.hpp>
#include <mgbase/scope/basic_unique_resource.hpp>

namespace mgcom {
namespace rma {

template <typename T>
struct default_local_delete
{
    void operator() (const mgcom::rma::local_ptr<T>& ptr)
    {
        mgcom::rma::deallocate(ptr);
    }
};

template <typename T, typename TD = default_local_delete<T>>
class unique_local_ptr;

namespace detail {

template <typename T, typename TD>
struct unique_local_ptr_traits
{
    typedef unique_local_ptr<T, TD>     derived_type;
    typedef mgcom::rma::local_ptr<T>    resource_type;
    typedef TD                          deleter_type;
};

} // namespace detail

template <typename T, typename TD>
class unique_local_ptr
    : public mgbase::basic_unique_resource<detail::unique_local_ptr_traits<T, TD>>
{
private:
    friend class mgbase::basic_unique_resource_access;
    
    bool is_owned() const MGBASE_NOEXCEPT
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
} // namespace mgcom

