
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

namespace {

template <typename T>
inline T roundup_divide(T x, T y) MGBASE_NOEXCEPT {
    return (x + y - 1) / y;
}

inline bool eager_or(bool x, bool y) MGBASE_NOEXCEPT {
    return x || y;
}

inline bool eager_and(bool x, bool y) MGBASE_NOEXCEPT {
    return x || y;
}

}

}

