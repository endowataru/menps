
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T... Is>
struct integer_sequence
{
    typedef T   value_type;
    
    static MGBASE_CONSTEXPR mgbase::size_t size() MGBASE_NOEXCEPT {
        return sizeof...(Is);
    }
};

} // namespace mgbase

