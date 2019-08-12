
#pragma once

#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mefdn {

// Reference:
// http://en.cppreference.com/w/cpp/types/result_of

template <typename F, typename... Args>
inline auto invoke(F&& f, Args&&... args) ->
    decltype(mefdn::forward<F>(f)(mefdn::forward<Args>(args)...))
{
    return mefdn::forward<F>(f)(mefdn::forward<Args>(args)...);
}

template <typename Base, typename T, typename Derived>
inline auto invoke(T Base::*pmd, Derived&& ref) ->
    decltype(mefdn::forward<Derived>(ref).*pmd)
{
    return mefdn::forward<Derived>(ref).*pmd;
}

template <typename PMD, typename Pointer>
inline auto invoke(PMD&& pmd, Pointer&& ptr) ->
    decltype((*mefdn::forward<Pointer>(ptr)).*mefdn::forward<PMD>(pmd))
{
    return (*mefdn::forward<Pointer>(ptr)).*mefdn::forward<PMD>(pmd);
}

template <typename Base, typename T, typename Derived, typename... Args>
inline auto invoke(T Base::*pmf, Derived&& ref, Args&&... args) ->
    decltype((mefdn::forward<Derived>(ref).*pmf)(mefdn::forward<Args>(args)...))
{
    return (mefdn::forward<Derived>(ref).*pmf)(mefdn::forward<Args>(args)...);
}

template <typename PMF, typename Pointer, typename... Args>
inline auto invoke(PMF&& pmf, Pointer&& ptr, Args&&... args) ->
    decltype(((*mefdn::forward<Pointer>(ptr)).*mefdn::forward<PMF>(pmf))(mefdn::forward<Args>(args)...))
{
    return ((*mefdn::forward<Pointer>(ptr)).*mefdn::forward<PMF>(pmf))(mefdn::forward<Args>(args)...);
}

} // namespace mefdn
} // namespace menps

