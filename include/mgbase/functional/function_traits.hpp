
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename Signature>
struct new_function_traits; // TODO: Temporarily using a different name

template <typename Signature>
struct new_function_traits<Signature*>
    : new_function_traits<Signature> { };

template <typename Result>
struct new_function_traits<Result ()>
{
    typedef Result  result_type;
};

template <typename Result, typename Arg1>
struct new_function_traits<Result (Arg1)>
{
    typedef Result  result_type;
    typedef Arg1    arg1_type;
};

template <typename Result, typename Arg1, typename Arg2>
struct new_function_traits<Result (Arg1, Arg2)>
{
    typedef Result  result_type;
    typedef Arg1    arg1_type;
    typedef Arg2    arg2_type;
};

template <typename F>
struct result_of;

template <typename Func>
struct result_of<Func ()>
{
    typedef typename new_function_traits<Func>::result_type  type;
};
template <typename Func, typename Arg1>
struct result_of<Func (Arg1)>
{
    typedef typename new_function_traits<Func>::result_type  type;
};
template <typename Func, typename Arg1, typename Arg2>
struct result_of<Func (Arg1, Arg2)>
{
    typedef typename new_function_traits<Func>::result_type  type;
};

} // namespace mgbase

