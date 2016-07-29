
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_CXX_SFINAE_EXPR

#include "invoke.hpp"
#include "make_void.hpp"
#include <mgbase/utility/declval.hpp>

namespace mgbase {

// Reference:
// http://en.cppreference.com/w/cpp/types/result_of

namespace detail {

template <typename, typename = void>
struct result_of {};

template <typename F, typename... Args>
struct result_of<
    F (Args...), void
/*,   typename mgbase::make_void<
        decltype(mgbase::invoke(mgbase::declval<F>(), mgbase::declval<Args>()...))
    >::type*/
> {
    typedef decltype(mgbase::invoke(mgbase::declval<F>(), mgbase::declval<Args>()...))
        type;
};

} // namespace detail
 
template <typename>
struct result_of;

template <typename F, typename... ArgTypes>
struct result_of<F (ArgTypes...)>
    : detail::result_of<F (ArgTypes...)> {};

} // namespace mgbase

#else

#include "is_member_function_pointer.hpp"
#include "is_member_object_pointer.hpp"
#include "is_base_of.hpp"
#include <mgbase/utility/declval.hpp>

namespace mgbase {

namespace detail {

template <bool IsReference, typename F, typename... Args>
struct result_of_member_function_helper { };

template <typename F, typename Derived, typename... Args>
struct result_of_member_function_helper<true, F, Derived, Args...>
{
    // (arg1.*f)(arg2, ...)
    typedef decltype(
        (mgbase::declval<Derived>().*(mgbase::declval<F>()))
        (mgbase::declval<Args>()...)
    ) type;
};
template <typename F, typename Derived, typename... Args>
struct result_of_member_function_helper<false, F, Derived, Args...>
{
    // ((*arg1).*f)(arg2, ...)
    typedef decltype(
        ((*mgbase::declval<Derived>()).*(mgbase::declval<F>()))
        (mgbase::declval<Args>()...)
    ) type;
};


template <bool IsReference, typename F, typename... Args>
struct result_of_member_object_helper { };

template <typename F, typename Derived>
struct result_of_member_object_helper<true, F, Derived>
{
    // arg1.*f
    typedef decltype(
        (mgbase::declval<Derived>().*(mgbase::declval<F>()))
    ) type;
};
template <typename F, typename Derived>
struct result_of_member_object_helper<false, F, Derived>
{
    // (*arg1).*f
    typedef decltype(
        (*mgbase::declval<Derived>()).*(mgbase::declval<F>())
    ) type;
};


template <bool IsMemberFunction, bool IsMemberObject, typename F, typename... Args>
struct result_of_helper { };

template <typename T, typename Base, typename Derived, typename... Args>
struct result_of_helper<true, false, T Base::*, Derived, Args...>
    : result_of_member_function_helper<
        mgbase::is_base_of<Base, Derived>::value, T Base::*, Derived, Args...
    > { };

template <typename T, typename Base, typename Derived>
struct result_of_helper<false, true, T Base::*, Derived>
    : result_of_member_object_helper<
        mgbase::is_base_of<Base, Derived>::value, T Base::*, Derived
    > { };

template <typename F, typename... Args>
struct result_of_helper<false, false, F, Args...>
{
    // f(arg1, arg2, ...)
    typedef decltype(
        mgbase::declval<F>()(mgbase::declval<Args>()...)
    ) type;
};

} // namespace detail

template <typename>
struct result_of;

template <typename F, typename... Args>
struct result_of<F (Args...)>
    : detail::result_of_helper<
        mgbase::is_member_function_pointer<F>::value
    ,   mgbase::is_member_object_pointer<F>::value
    ,   F
    ,   Args...
    >
    { };

} // namespace mgbase

#endif

namespace mgbase {

// non-standard flattened version
template <typename... Args>
struct invoke_of;

template <typename F, typename... Args>
struct invoke_of<F, Args...>
    : result_of<F (Args...)> { };

} // namespace mgbase

