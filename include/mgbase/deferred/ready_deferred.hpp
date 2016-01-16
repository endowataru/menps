
#pragma once

#include <mgbase/value_wrapper.hpp>

namespace mgbase {

namespace /*unnamed*/ {

inline MGBASE_ALWAYS_INLINE value_wrapper<void> make_ready_deferred() MGBASE_NOEXCEPT {
    return wrap_void();
}

template <typename T>
inline MGBASE_ALWAYS_INLINE value_wrapper<T> make_ready_deferred(const T& val) {
    return wrap_value(val);
}

} // unnamed namespace

} // namespace mgbase

