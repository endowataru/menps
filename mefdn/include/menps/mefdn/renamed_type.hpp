
#pragma once

// TODO: Deprecated

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

template <typename Defined>
struct renamed_type {
    typedef Defined     type;
};

#define MEFDN_DEFINE_RENAMED_TYPE(Defined, Actual)  \
    namespace mgbase { \
        template <> \
        struct renamed_type<Defined> { \
            typedef Actual  type; \
        }; \
    }

template <typename T>
struct get_actual_type
{
    typedef typename renamed_type<T>::type    type;
};

template <typename T>
struct get_actual_type<const T>
{
    typedef const typename renamed_type<T>::type  type;
};

} // namespace mefdn
} // namespace menps

