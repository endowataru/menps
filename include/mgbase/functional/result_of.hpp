
#pragma once

#ifdef MGBASE_CPP11_SUPPORTED

#include <type_traits>

namespace mgbase {

using std::result_of;

} // namespace mgbase

#else

namespace mgbase {

template <typename F>
struct result_of;

template <typename Func>
struct result_of<Func ()>
{
    typedef typename function_traits<Func>::result_type  type;
};
template <typename Func, typename Arg1>
struct result_of<Func (Arg1)>
{
    typedef typename function_traits<Func>::result_type  type;
};
template <typename Func, typename Arg1, typename Arg2>
struct result_of<Func (Arg1, Arg2)>
{
    typedef typename function_traits<Func>::result_type  type;
};

} // namespace mgbase

#endif

