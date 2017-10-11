
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename Policy>
class crtp_base
{
    typedef typename Policy::derived_type   derived_type;
    
public:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    const derived_type& derived() const MGBASE_NOEXCEPT {
        return static_cast<const derived_type&>(*this);
    }
};

} // namespace mgbase

// macro-based

#define MGBASE_POLICY_BASED_CRTP(Policy) \
    private:\
        typedef typename Policy::derived_type   derived_type; \
        \
        derived_type& derived() MGBASE_NOEXCEPT { \
            return static_cast<derived_type&>(*this); \
        } \
        const derived_type& derived() const MGBASE_NOEXCEPT { \
            return static_cast<const derived_type&>(*this); \
        }


