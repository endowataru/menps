
#pragma once

#include "bound_function.h"
#include <mgbase/assert.hpp>

namespace mgbase {

typedef mgbase_untyped_bound_function  untyped_bound_function;

namespace detail {

inline bool equal(const untyped_bound_function& f0, const untyped_bound_function& f1)
{
    return f0.func == f1.func && f0.arg1 == f1.arg1;
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
    
    template <typename Signature, Signature* Func>
    static bound_function create()
    {
        struct handler {
            static result_type transfer(void* /*arg1*/) {
                return Func();
            }
        };
        
        bound_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = MGBASE_NULLPTR;
        
        return f;
    }
    
    template <typename Signature, Signature* Func, typename Arg1>
    static bound_function create(Arg1* arg1)
    {
        struct handler {
            static result_type transfer(void* a1) {
                return Func(*static_cast<Arg1*>(a1));
            }
        };
        
        bound_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = arg1;
        
        return f;
    }
    
    result_type operator() () {
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
    template <typename Signature, Signature* Func, typename Arg1>
    static bound_function create(Arg1* arg1)
    {
        struct handler {
            static result_type transfer(void* a1, Arg2 arg2) {
                return Func(*static_cast<Arg1*>(a1), arg2);
            }
        };
        
        bound_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = arg1;
        
        return f;
    }
    
    template <typename Signature, Signature* Func>
    static bound_function create_unbound()
    {
        struct handler {
            static result_type transfer(void* /*ignored*/, Arg2 arg2) {
                return Func(arg2);
            }
        };
        
        bound_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_bound_function_func_ptr_t>(handler::transfer);
        //f.untyped_.arg1 = MGBASE_NULLPTR; // ignored
        
        return f;
    }
    
    result_type operator() (Arg2 arg2) {
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

template <typename Signature, Signature* Func>
inline bound_function<Signature> make_unbound_function() {
    return bound_function<Signature>::template create_unbound<Signature, Func>();
}

} // unnamed namespace

} // namespace mgbase

