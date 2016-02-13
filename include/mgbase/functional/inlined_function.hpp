
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

#define MGBASE_MAKE_INLINED_FUNCTION(f) (::mgbase::detail::make_signature_deducer(f).make_inlined<f>())
#define MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(f) (::mgbase::detail::make_signature_deducer(f).template make_inlined<f>())

template <typename Signature, Signature Func>
struct new_function_traits< inlined_function<Signature, Func> >
    : new_function_traits<Signature> { };

} // namespace mgbase

