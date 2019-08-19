
#pragma once

#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

// TODO : Is this unnecessary with enable_if?

template <
    typename Policy
,   bool Dereferencable =
        // resource_type is a pointer
        mefdn::is_pointer<typename Policy::resource_type>::value
        &&
        // resource_type is not void*
        ! mefdn::is_void<
            typename mefdn::remove_pointer_t<typename Policy::resource_type>
        >::value
>
class basic_dereferencable
{ };

template <typename Policy>
class basic_dereferencable<Policy, true>
{
    MEFDN_DEFINE_DERIVED(Policy)
    
    typedef typename mefdn::add_lvalue_reference_t<
        typename mefdn::remove_pointer_t<
            typename Policy::resource_type
        >
    >
    lv_ref_resource_type;
    
public:
    lv_ref_resource_type operator * () const noexcept {
        return * this->derived().get();
    }
};

} // namespace mefdn
} // namespace menps

