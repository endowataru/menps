
#pragma once

#include <mgbase/type_traits/invoke.hpp>

namespace mgbase {

template <typename T, T Value>
struct functional_constant
{
    template <typename... Args>
    auto operator() (Args&&... args) const volatile
        -> decltype(mgbase::invoke(Value, std::forward<Args>(args)...))
    {
        return mgbase::invoke(Value, std::forward<Args>(args)...);
    }
};

#define MGBASE_FUNCTIONAL_CONSTANT(...) \
    (::mgbase::functional_constant<decltype(__VA_ARGS__), __VA_ARGS__>{})

} // namespace mgbase


