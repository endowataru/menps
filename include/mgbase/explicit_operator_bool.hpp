
#pragma once

#include <mgbase/lang.hpp>

// See also: explicit operator bool in Boost.Core

#ifdef MGBASE_CXX11_EXPLICIT_CONVERSION_OPERATORS_SUPPORTED

#define MGBASE_EXPLICIT_OPERATOR_BOOL() \
    explicit operator bool() const { \
        return ! this->operator!(); \
    }

#else

namespace mgbase {

namespace detail {

struct unspecified_bool
{
    struct operators_not_allowed;
    void true_value(operators_not_allowed*);
};

typedef void (unspecified_bool::*unspecified_bool_type)(unspecified_bool::operators_not_allowed*);

} // namespace detail

} // namespace mgbase

#define MGBASE_EXPLICIT_OPERATOR_BOOL() \
    operator ::mgbase::detail::unspecified_bool_type() const { \
        return ! this->operator!() \
            ? &::mgbase::detail::unspecified_bool::true_value \
            : MGBASE_NULLPTR; \
    }


#endif

