
#error

#if 0

#pragma once

#include "core.hpp"

#ifdef MGBASE_ENABLE_EMULATE_MOVE

namespace mgbase {

// move

template <typename T>
MGBASE_ALWAYS_INLINE rv<T>& move(T& x) MGBASE_NOEXCEPT {
    // TODO: use addressof
    return * detail::move_to_rv_cast<rv<T>*>(& x);
}

template <typename T>
MGBASE_ALWAYS_INLINE rv<T>& move(rv<T>& x) MGBASE_NOEXCEPT {
    return x;
}

// forward

template <typename T>
MGBASE_ALWAYS_INLINE
typename mgbase::enable_if<
    detail::is_rv<T>::value // T is rv
,   rv<T>&
>::type
forward(const typename mgbase::identity<T>::type& x) MGBASE_NOEXCEPT {
    return const_cast<T&>(x);
}

template <typename T>
MGBASE_ALWAYS_INLINE
typename mgbase::enable_if<
    ! detail::is_rv<T>::value // T is not rv
,   const T&
>::type
forward(const typename mgbase::identity<T>::type& x) MGBASE_NOEXCEPT {
    return x;
}

} // namespace mgbase

#endif

#endif

