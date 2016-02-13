
#pragma once

#include "callback_function.hpp"
#include <mgbase/value_wrapper.hpp>
#include <mgbase/type_traits.hpp>

namespace mgbase {

namespace detail {

template <typename Derived, typename Func, typename Arg1>
class bound1st_base
{
public:
    MGBASE_ALWAYS_INLINE static Derived create(Func func, Arg1 arg1) MGBASE_NOEXCEPT {
        Derived f;
        f.func_ = func;
        f.arg1_ = arg1;
        return f;
    }
    
    MGBASE_ALWAYS_INLINE Func func() const { return func_; }
    MGBASE_ALWAYS_INLINE Arg1 arg1() const { return arg1_; }
    
private:
    Func func_;
    Arg1 arg1_;
};

} // namespace detail

template <typename Func, typename Arg1>
class bound1st_of_1
    : public detail::bound1st_base<bound1st_of_1<Func, Arg1>, Func, Arg1>
{
public:
    MGBASE_ALWAYS_INLINE typename mgbase::result_of<Func (Arg1)>::type
    operator () () {
        return this->func()(this->arg1());
    }
};

template <typename Func, typename Arg1>
class bound1st_of_2
    : public detail::bound1st_base<bound1st_of_2<Func, Arg1>, Func, Arg1>
{
public:
    template <typename Arg2>
    MGBASE_ALWAYS_INLINE typename mgbase::result_of<Func (Arg1, Arg2)>::type
    operator () (Arg2 arg2) {
        return this->func()(this->arg1(), arg2);
    }
};

template <typename Func, typename Arg1>
MGBASE_ALWAYS_INLINE bound1st_of_1<Func, Arg1> bind1st_of_1(Func func, Arg1 arg1) MGBASE_NOEXCEPT {
    return bound1st_of_1<Func, Arg1>::create(func, arg1);
}

template <typename Func, typename Arg1>
MGBASE_ALWAYS_INLINE bound1st_of_2<Func, Arg1> bind1st_of_2(Func func, Arg1 arg1) MGBASE_NOEXCEPT {
    return bound1st_of_2<Func, Arg1>::create(func, arg1);
}

} // namespace mgbase

