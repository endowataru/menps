
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
struct add_volatile {
    typedef volatile T type;
};

} // namespace mgbase


