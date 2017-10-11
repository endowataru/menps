
#pragma once

#include "function_traits.hpp"

namespace mgbase {

// Reference:
// http://d.hatena.ne.jp/Cryolite/01000831

template <typename Signature, Signature Func>
struct inlined_function;

// Regular functions

template <typename R, R (Func)()>
struct inlined_function<R (*)(), Func>
{
    MGBASE_ALWAYS_INLINE R operator() () const {
        return Func();
    }
};

template <typename R, typename A1, R (Func)(A1)>
struct inlined_function<R (*)(A1), Func>
{
    MGBASE_ALWAYS_INLINE R operator() (A1 a1) const {
        return Func(a1);
    }
};

template <typename R, typename A1, typename A2, R (Func)(A1, A2)>
struct inlined_function<R (*)(A1, A2), Func>
{
    MGBASE_ALWAYS_INLINE R operator() (A1 a1, A2 a2) const {
        return Func(a1, a2);
    }
};

template <typename R, typename A1, typename A2, typename A3, R (Func)(A1, A2, A3)>
struct inlined_function<R (*)(A1, A2, A3), Func>
{
    MGBASE_ALWAYS_INLINE R operator() (A1 a1, A2 a2, A3 a3) const {
        return Func(a1, a2, a3);
    }
};

// Member functions

template <typename Result, typename Class, Result (Class::*Func)()>
struct inlined_function<Result (Class::*)(), Func>
{
    MGBASE_ALWAYS_INLINE Result operator() (Class& self) const {
        return (self.*Func)();
    }
};

namespace detail {

template <typename Signature>
struct signature_deducer {
    template <Signature Func>
    MGBASE_ALWAYS_INLINE inlined_function<Signature, Func> make_inlined() const MGBASE_NOEXCEPT {
        return inlined_function<Signature, Func>();
    }
};

template <typename Signature>
MGBASE_ALWAYS_INLINE signature_deducer<Signature>
make_signature_deducer(Signature /*unused*/) MGBASE_NOEXCEPT {
    return signature_deducer<Signature>();
}

} // namespace detail

// Factory functions defined as macros

#define MGBASE_MAKE_INLINED_FUNCTION(...) \
    (::mgbase::detail::make_signature_deducer(__VA_ARGS__).make_inlined<__VA_ARGS__>())

#define MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(...) \
    (::mgbase::detail::make_signature_deducer(__VA_ARGS__).template make_inlined<__VA_ARGS__>())

/*
    Note: Using a parenthesis for non-type template arguments becomes an error in GCC.
    
    GCC:
        error: ‘void f()’ cannot appear in a constant-expression
        error: ‘&’ cannot appear in a constant-expression
        error: template argument 1 is invalid
        error: invalid type in declaration before ‘;’ token
    
    Clang:
        warning: address non-type template argument cannot be surrounded by parentheses
    
    To avoid encountering this error, MGBASE_MAKE_INLINED_FUNCTION uses __VA_ARGS__
    to pass an function pointer argument to the deducer and factory function.
    
    The users of these macros must not wrap the argument by a parentheses
    even if it contains commas (,).
*/

// Template specialization for function_traits

template <typename Signature, Signature Func>
struct function_traits< inlined_function<Signature, Func> >
    : function_traits<Signature> { };

} // namespace mgbase

