
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename Signature>
struct function_traits;

template <typename R>
struct function_traits<R ()>
{
    typedef R   result_type;
};

template <typename R, typename A0>
struct function_traits<R (A0)>
{
    typedef R   result_type;
    typedef A0  arg0_type;
};

template <typename R, typename A0, typename A1>
struct function_traits<R (A0, A1)>
{
    typedef R   result_type;
    typedef A0  arg0_type;
    typedef A1  arg1_type;
};

} // namespace mgbase

