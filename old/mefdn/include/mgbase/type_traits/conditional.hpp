
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <bool Condition, typename True, typename False>
struct conditional {
    typedef True type;
};

template <typename True, typename False>
struct conditional<false, True, False> {
    typedef False type;
};

} // namespace mgbase

