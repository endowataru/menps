
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class unique_local_ptr_deleter
{
    using allocator_type = typename P::allocator_type;
    using resource_type = typename P::resource_type;
    
public:
    /*implicit*/ unique_local_ptr_deleter(allocator_type& alloc)
        : alloc_(alloc) { }
    
    void operator() (resource_type p) const {
        alloc_.untyped_deallocate(p);
    }
    
private:
    allocator_type& alloc_;
};

template <typename P>
class basic_unique_local_ptr
    : public mefdn::basic_unique_resource<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = mefdn::basic_unique_resource<P>;
    
public:
    using base::base;
    
private:
    friend mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept {
        return this->get_resource();
    }
    
    void set_unowned() noexcept {
        this->get_resource() = nullptr;
    }
    void set_owned() noexcept {
        // do nothing
    }
};

} // namespace mecom2
} // namespace menps

