
#pragma once

#include "remove_reference.hpp"
#include "conditional.hpp"
#include "is_array.hpp"
#include "remove_extent.hpp"
#include "is_function.hpp"
#include "add_pointer.hpp"
#include "remove_cv.hpp"

namespace mgbase {

template <typename T>
struct decay
{
private:
    typedef typename mgbase::remove_reference<T>::type U;

public:
    typedef typename
    mgbase::conditional< 
        mgbase::is_array<U>::value
    ,   typename mgbase::remove_extent<U>::type*
    ,   typename mgbase::conditional< 
            mgbase::is_function<U>::value
        ,   typename mgbase::add_pointer<U>::type
        ,   typename mgbase::remove_cv<U>::type
        >::type
    >::type type;
};

} // namespace mgbase

