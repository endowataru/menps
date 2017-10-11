
#pragma once

#include <mgbase/lang.hpp>
#include <memory>

namespace mgbase {

#ifdef MGBASE_CXX11_ADDRESSOF_SUPPORTED

using std::addressof;

#else

template <typename T>
inline T* addressof(T& arg) MGBASE_NOEXCEPT
{
    return reinterpret_cast<T*>(
        &const_cast<char&>(
            reinterpret_cast<const volatile char&>(arg)
        )
    );
}

#endif

} // namespace mgbase

