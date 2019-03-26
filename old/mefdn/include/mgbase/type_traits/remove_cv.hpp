
#pragma once

#include "remove_volatile.hpp"
#include "remove_const.hpp"

namespace mgbase {

template <typename T>
struct remove_cv {
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

} // namespace mgbase

