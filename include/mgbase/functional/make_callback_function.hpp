
#pragma once

#include "callback_function.hpp"
#include "bind_arg.hpp"

namespace mgbase {

// make_callback_function
//  Do type-erasure for inlined_function.

namespace detail {

template <typename Result, Result (* Func)()>
MGBASE_ALWAYS_INLINE Result unbound(void* /*unused*/) {
    return Func();
}

template <typename Result, typename Arg1, Result (* Func)(Arg1)>
MGBASE_ALWAYS_INLINE Result unbound(void* /*unused*/, Arg1 arg1) {
    return Func(arg1);
}

} // namespace detail

template <typename Result, Result (* Func)()>
MGBASE_ALWAYS_INLINE callback_function<Result ()>
make_callback_function(inlined_function<Result (*)(), Func> /*unused*/)
{
    Result (* const f)(void*) = &detail::unbound<Result, Func>;
    
    return callback_function<Result ()>::create(f, MGBASE_NULLPTR);
}

template <typename Result, typename Arg1, Result (* Func)(Arg1)>
MGBASE_ALWAYS_INLINE callback_function<Result (Arg1)>
make_callback_function(inlined_function<Result (*)(Arg1), Func> /*unused*/)
{
    Result (* const f)(void*, Arg1) = &detail::unbound<Result, Arg1, Func>;
    
    return callback_function<Result (Arg1)>::create(f, MGBASE_NULLPTR);
}

namespace detail {

template <typename Result, typename Arg1, Result (* Func)(Arg1&)>
MGBASE_ALWAYS_INLINE Result pass_ref1(void* ptr) {
    Arg1& arg1 = *static_cast<Arg1*>(ptr);
    return Func(arg1);
}

template <typename Result, typename Arg1, typename Arg2, Result (* Func)(Arg1&, Arg2)>
MGBASE_ALWAYS_INLINE Result pass_ref1(void* ptr, Arg2 arg2) {
    Arg1& arg1 = *static_cast<Arg1*>(ptr);
    return Func(arg1, arg2);
}

} // namespace detail

template <typename Result, typename Arg1, Result (* Func)(Arg1&), typename A1>
MGBASE_ALWAYS_INLINE callback_function<Result ()>
make_callback_function(bound1st_of_1<inlined_function<Result (*)(Arg1&), Func>, A1> bound)
{
    Result (* const func)(void*) = &detail::pass_ref1<Result, Arg1, Func>;
    Arg1& arg1 = bound.arg1();
    
    // Implicitly cast to void* first.
    const volatile void* arg1_cv_ptr = &arg1;
    // Remove CV.
    void* const arg1_ptr = const_cast<void*>(arg1_cv_ptr);
    
    return callback_function<Result ()>::create(func, arg1_ptr);
}

template <typename Result, typename Arg1, typename Arg2, Result (* Func)(Arg1&, Arg2), typename A1>
MGBASE_ALWAYS_INLINE callback_function<Result (Arg2)>
make_callback_function(bound1st_of_2<inlined_function<Result (*)(Arg1&, Arg2), Func>, A1> bound)
{
    Result (* const func)(void*, Arg2) = &detail::pass_ref1<Result, Arg1, Arg2, Func>;
    Arg1& arg1 = bound.arg1();
    
    // Implicitly cast to void* first.
    const volatile void* arg1_cv_ptr = &arg1;
    // Remove CV.
    void* const arg1_ptr = const_cast<void*>(arg1_cv_ptr);
    
    return callback_function<Result (Arg2)>::create(func, arg1_ptr);
}

namespace detail {

template <typename Result, typename Self, Result (Self::*Func)()>
MGBASE_ALWAYS_INLINE Result pass_this(void* ptr) {
    Self& self = *static_cast<Self*>(ptr);
    return (self.*Func)();
}

} // namespace detail


} // namespace mgbase

