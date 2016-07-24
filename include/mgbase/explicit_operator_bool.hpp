
#pragma once

#include <mgbase/lang.hpp>

// See also: explicit operator bool in Boost.Core

#ifdef MGBASE_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS

namespace mgbase {

namespace detail {

struct unspecified_bool
{
    struct operators_not_allowed;
    static void true_value(operators_not_allowed*);
};

typedef void (unspecified_bool::*unspecified_bool_type)(operators_not_allowed*);

} // namespace detail

} // namespace mgbase

#define MGBASE_EXPLICIT_OPERATOR_BOOL() \
    operator ::mgbase::detail::unspecified_bool_type() const { \
        return ! this->operator!() \
            ? &::mgbase::detail::unspecified_bool::true_value \
            : ::mgbase::detail::unspecified_bool{}; \
    }

#else

#define MGBASE_EXPLICIT_OPERATOR_BOOL() \
    explicit operator bool() const { \
        return ! this->operator!(); \
    }

#endif

