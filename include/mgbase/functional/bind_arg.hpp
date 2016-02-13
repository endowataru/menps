
#pragma once

#include "callback_function.hpp"
#include <mgbase/value_wrapper.hpp>
#include <mgbase/type_traits.hpp>

namespace mgbase {

template <typename Func, typename Arg1>
class bound_function_arg1;

namespace detail {

template <typename Func, typename Arg1, bool ZeroArg>
class bound_function_arg1_base
{
public:
    template <typename X> void operator () (); // never defined
};

template <typename Func, typename Arg1>
class bound_function_arg1_base<Func, Arg1, true>
{
    typedef bound_function_arg1<Func, Arg1> derived_type;
    
public:
    MGBASE_ALWAYS_INLINE typename mgbase::result_of<Func (Arg1)>::type
    operator () () {
        return derived().func()(derived().arg1());
    }
    // TODO
    /*MGBASE_ALWAYS_INLINE typename mgbase::result_of<const Func (Arg1)>::type
    operator () () const {
        return derived().func_(derived().arg1_.get());
    }*/
    
private:
          derived_type& derived()       { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const { return static_cast<const derived_type&>(*this); }
};

} // namespace detail

template <typename Func, typename Arg1>
class bound_function_arg1
    : public detail::bound_function_arg1_base<Func, Arg1, mgbase::is_callable<Func (Arg1)>::value>
{
    typedef detail::bound_function_arg1_base<Func, Arg1, mgbase::is_callable<Func (Arg1)>::value>   base;
    
public:
    MGBASE_ALWAYS_INLINE static bound_function_arg1 create(Func func, Arg1 arg1) MGBASE_NOEXCEPT {
        bound_function_arg1 f;
        f.func_ = func;
        f.arg1_ = arg1;
        return f;
    }
    
    using base::operator();
    
    template <typename Arg2>
    MGBASE_ALWAYS_INLINE typename mgbase::result_of<Func (Arg1, Arg2)>::type
    operator () (Arg2 arg2) {
        return func_(arg1_.get(), arg2);
    }
    // TODO
    /*template <typename Arg2>
    MGBASE_ALWAYS_INLINE typename mgbase::result_of<const Func (Arg1, Arg2)>::type
    operator () (Arg2 arg2) const {
        return func_(arg1_.get(), arg2);
    }*/
    
    MGBASE_ALWAYS_INLINE Func func() const { return func_; }
    MGBASE_ALWAYS_INLINE Arg1 arg1() const { return arg1_.get(); }
    
private:
    Func func_;
    mgbase::value_wrapper<Arg1> arg1_;
};

template <typename Func, typename Arg1>
MGBASE_ALWAYS_INLINE bound_function_arg1<Func, Arg1> bind_copy1(Func func, Arg1 arg1) MGBASE_NOEXCEPT {
    return bound_function_arg1<Func, Arg1>::create(func, arg1);
}

template <typename Func, typename Arg1>
MGBASE_ALWAYS_INLINE bound_function_arg1<Func, Arg1&> bind_ref1(Func func, Arg1& arg1) MGBASE_NOEXCEPT {
    return bound_function_arg1<Func, Arg1&>::create(func, arg1);
}

#if 0
template <typename Func, typename Self>
class bound_function_this
{
public:
    MGBASE_ALWAYS_INLINE static bound_function_this create(Func func, Self& self) MGBASE_NOEXCEPT {
        bound_function_this f;
        f.func_ = func;
        f.self_ = self;
        return f;
    }
    
private:
    Func    func_;
    Self*   self_;
};
#endif

} // namespace mgbase

