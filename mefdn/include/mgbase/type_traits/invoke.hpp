
#pragma once

#include <mgbase/utility/forward.hpp>

#ifdef MGBASE_CXX_SFINAE_EXPR

namespace mgbase {

// Reference:
// http://en.cppreference.com/w/cpp/types/result_of

template <typename F, typename... Args>
inline auto invoke(F&& f, Args&&... args) ->
    decltype(mgbase::forward<F>(f)(mgbase::forward<Args>(args)...))
{
    return mgbase::forward<F>(f)(mgbase::forward<Args>(args)...);
}

template <typename Base, typename T, typename Derived>
inline auto invoke(T Base::*pmd, Derived&& ref) ->
    decltype(mgbase::forward<Derived>(ref).*pmd)
{
    return mgbase::forward<Derived>(ref).*pmd;
}

template <typename PMD, typename Pointer>
inline auto invoke(PMD&& pmd, Pointer&& ptr) ->
    decltype((*mgbase::forward<Pointer>(ptr)).*mgbase::forward<PMD>(pmd))
{
    return (*mgbase::forward<Pointer>(ptr)).*mgbase::forward<PMD>(pmd);
}

template <typename Base, typename T, typename Derived, typename... Args>
inline auto invoke(T Base::*pmf, Derived&& ref, Args&&... args) ->
    decltype((mgbase::forward<Derived>(ref).*pmf)(mgbase::forward<Args>(args)...))
{
    return (mgbase::forward<Derived>(ref).*pmf)(mgbase::forward<Args>(args)...);
}

template <typename PMF, typename Pointer, typename... Args>
inline auto invoke(PMF&& pmf, Pointer&& ptr, Args&&... args) ->
    decltype(((*mgbase::forward<Pointer>(ptr)).*mgbase::forward<PMF>(pmf))(mgbase::forward<Args>(args)...))
{
    return ((*mgbase::forward<Pointer>(ptr)).*mgbase::forward<PMF>(pmf))(mgbase::forward<Args>(args)...);
}

} // namespace mgbase

#else

#include "result_of.hpp"

namespace mgbase {

template <typename F, typename... Args>
typename mgbase::result_of<
    F (Args...)
>::type
invoke(F&& f, Args&&... args) {
    return mgbase::forward<F>(f)(mgbase::forward<Args>(args)...);
}

} // namespace mgbase

#endif

