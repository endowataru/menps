
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename Signature>
struct function_traits; // TODO: Temporarily using a different name

template <typename Signature>
struct function_traits<Signature*>
    : function_traits<Signature> { };

template <typename Result>
struct function_traits<Result ()>
{
    typedef Result  result_type;
};

template <typename Result, typename Arg1>
struct function_traits<Result (Arg1)>
{
    typedef Result  result_type;
    typedef Arg1    arg1_type;
};

template <typename Result, typename Arg1, typename Arg2>
struct function_traits<Result (Arg1, Arg2)>
{
    typedef Result  result_type;
    typedef Arg1    arg1_type;
    typedef Arg2    arg2_type;
};

} // namespace mgbase

