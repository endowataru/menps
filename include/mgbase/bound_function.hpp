
#pragma once

#include "bound_function.h"
#include <mgbase/assert.hpp>
#include <mgbase/function_traits.hpp>

namespace mgbase {

typedef mgbase_untyped_bound_function  untyped_bound_function;

namespace detail {

inline bool equal(const untyped_bound_function& f0, const untyped_bound_function& f1)
{
    return f0.func == f1.func && f0.arg1 == f1.arg1;
}

template <typename Result, Result (*Func)()>
inline Result unbound(void* /*a1*/) {
    return Func();
}

template <typename Result, typename Arg1, Result (*Func)(Arg1&)>
inline Result bind_ref(void* arg1) {
    return Func(*static_cast<Arg1*>(arg1));
}

template <typename Result, typename Arg1, Result (Arg1::*Func)()>
inline Result bind_this(void* arg1) {
    return (static_cast<Arg1*>(arg1)->*Func)();
}

template <typename Result, typename Arg2, Result (*Func)(Arg2&)>
inline Result unbound(void* /*ignored*/, Arg2 arg2) {
    return Func(arg2);
}

template <typename Result, typename Arg1, typename Arg2, Result (*Func)(Arg1&, Arg2)>
inline Result bind_ref(void* arg1, Arg2 arg2) {
    return Func(*static_cast<Arg1*>(arg1), arg2);
}

} // namespace detail

template <typename Signature>
class bound_function;

template <typename Result>
class bound_function<Result ()>
{
    typedef Result  result_type;
    
public:
    static bound_function create_empty()
    {
        bound_function f;
        f.untyped_.func = MGBASE_NULLPTR;
        f.untyped_.arg1 = MGBASE_NULLPTR;
        return f;
    }
    
    template <Result (*Func)()>
    static bound_function create()
    {
        bound_function f;
        // Note: Fujitsu compiler requires assignment of template functions to convert into function pointers
        Result (* const func)(void*) = &detail::unbound<Result, Func>;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(func);
        f.untyped_.arg1 = MGBASE_NULLPTR;
        
        return f;
    }
    
    template <typename Signature, Signature* Func, typename Arg1>
    static bound_function create(Arg1* const arg1)
    {
        bound_function f;
        Result (* const func)(void*) = &detail::bind_ref<Result, Arg1, Func>;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(func);
        f.untyped_.arg1 = const_cast<void*>(static_cast<const void*>(arg1));
        
        return f;
    }
    
    template <typename Signature, Signature Func, typename Arg1>
    static bound_function create_member(Arg1* const arg1)
    {
        bound_function f;
        Result (* const func)(void*) = &detail::bind_this<Result, Arg1, Func>;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(func);
        f.untyped_.arg1 = const_cast<void*>(static_cast<const void*>(arg1));
        
        return f;
    }
    
    result_type operator() () const {
        MGBASE_ASSERT(untyped_.func != MGBASE_NULLPTR);
        return reinterpret_cast<result_type (*)(void*)>(untyped_.func)(untyped_.arg1);
    }
    
    bool operator == (const bound_function& func) const MGBASE_NOEXCEPT {
        return detail::equal(untyped_, func.untyped_);
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return untyped_.func == MGBASE_NULLPTR;
    }
    
private:
    mgbase_untyped_bound_function untyped_;
};

template <typename Result, typename Arg2>
class bound_function<Result (Arg2)>
{
    typedef Result  result_type;
    
public:
    template <typename Signature, Signature* Func>
    static bound_function create_unbound()
    {
        bound_function f;
        Result (* const func)(void*, Arg2) = &detail::unbound<Result, Arg2, Func>;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(func);
        f.untyped_.arg1 = MGBASE_NULLPTR; // ignored, but reset to zero for debugging
        
        return f;
    }
    
    template <typename Signature, Signature* Func, typename Arg1>
    static bound_function create(Arg1* arg1)
    {
        bound_function f;
        Result (* const func)(void*, Arg2) = &detail::bind_ref<Result, Arg1, Arg2, Func>;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(func);
        f.untyped_.arg1 = const_cast<void*>(static_cast<const void*>(arg1));
        
        return f;
    }
    
    result_type operator() (Arg2 arg2) const {
        MGBASE_ASSERT(untyped_.func != MGBASE_NULLPTR);
        return reinterpret_cast<result_type (*)(void*, Arg2)>(untyped_.func)(untyped_.arg1, arg2);
    }
    
    bool operator == (const bound_function& func) const MGBASE_NOEXCEPT {
        return detail::equal(untyped_, func.untyped_);
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return untyped_.func == MGBASE_NULLPTR;
    }
    
private:
    untyped_bound_function untyped_;
};

namespace detail {

template <typename S>
struct bound_function_signature;

template <typename R>
struct bound_function_signature<R ()>
{
    typedef R (type)();
};

template <typename R, typename A1>
struct bound_function_signature<R (A1)>
{
    typedef R (type)();
};

template <typename R, typename A1, typename A2>
struct bound_function_signature<R (A1, A2)>
{
    typedef R (type)(A2);
};

template <typename R, typename A1>
struct bound_function_signature<R (A1::*)()>
{
    typedef R (type)();
};

} // namespace detail 

namespace /*unnamed*/ {

template <typename Signature, Signature* Func>
inline bound_function<typename detail::bound_function_signature<Signature>::type> make_bound_function() {
    return bound_function<typename detail::bound_function_signature<Signature>::type>::template create<Signature, Func>();
}

template <typename Signature, Signature* Func, typename Arg1>
inline bound_function<typename detail::bound_function_signature<Signature>::type> make_bound_function(Arg1* arg1) {
    return bound_function<typename detail::bound_function_signature<Signature>::type>::template create<Signature, Func>(arg1);
}

template <typename Signature, Signature Func, typename Arg1>
inline bound_function<typename detail::bound_function_signature<Signature>::type> make_bound_member_function(Arg1* arg1) {
    return bound_function<typename detail::bound_function_signature<Signature>::type>::template create_member<Signature, Func>(arg1);
}

template <typename Signature, Signature* Func>
inline bound_function<Signature> make_unbound_function() {
    return bound_function<Signature>::template create_unbound<Signature, Func>();
}

} // unnamed namespace

} // namespace mgbase

