
#pragma once

#include <menps/mefdn/lang.hpp>

// TODO: Deprecate

namespace menps {
namespace mefdn {

template <typename Policy>
class MEFDN_DEPRECATED crtp_base
{
    typedef typename Policy::derived_type   derived_type;
    
public:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
    const derived_type& derived() const noexcept {
        return static_cast<const derived_type&>(*this);
    }
};

} // namespace mefdn
} // namespace mgbase

// macro-based

#define MEFDN_POLICY_BASED_CRTP(Policy) \
    private:\
        typedef typename Policy::derived_type   derived_type; \
        \
        derived_type& derived() noexcept { \
            return static_cast<derived_type&>(*this); \
        } \
        const derived_type& derived() const noexcept { \
            return static_cast<const derived_type&>(*this); \
        }


