
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/type_traits/is_pointer.hpp>
#include <mgbase/type_traits/is_void.hpp>
#include <mgbase/type_traits/add_lvalue_reference.hpp>
#include <mgbase/type_traits/remove_pointer.hpp>
#include <mgbase/lang.hpp>

namespace mgbase {

template <
    typename Policy
,   bool Dereferencable =
        // resource_type is a pointer
        mgbase::is_pointer<typename Policy::resource_type>::value
        &&
        // resource_type is not void*
        ! mgbase::is_void<
            typename mgbase::remove_pointer<typename Policy::resource_type>::type
        >::value
>
class basic_dereferencable
{ };

template <typename Policy>
class basic_dereferencable<Policy, true>
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef typename mgbase::add_lvalue_reference<
        typename mgbase::remove_pointer<
            typename Policy::resource_type
        >::type
    >::type
    lv_ref_resource_type;
    
public:
    lv_ref_resource_type operator * () const MGBASE_NOEXCEPT {
        return * this->derived().get();
    }
};

} // namespace mgbase

