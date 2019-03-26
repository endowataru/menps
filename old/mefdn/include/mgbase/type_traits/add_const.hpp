
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
struct add_const {
    typedef const T type;
};

} // namespace mgbase


