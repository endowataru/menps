
#pragma once

#include "add_const.hpp"
#include "add_volatile.hpp"

namespace mgbase {

template <typename T>
struct add_cv {
    typedef typename mgbase::add_volatile<
        typename mgbase::add_const<T>::type
    >::type type;
};

} // namespace mgbase


