
#pragma once

#include "binded_function.h"
#include <mgbase/assert.hpp>

namespace mgbase {

typedef mgbase_untyped_binded_function  untyped_binded_function;

namespace detail {

inline bool equal(const untyped_binded_function& f0, const untyped_binded_function& f1)
{
    return f0.func == f1.func && f0.arg1 == f1.arg1;
}

} // namespace detail

template <typename Signature>
class binded_function;

template <typename Result>
class binded_function<Result ()>
{
    typedef Result  result_type;
    
public:
    static binded_function create_empty()
    {
        binded_function f;
        f.untyped_.func = MGBASE_NULLPTR;
        f.untyped_.arg1 = MGBASE_NULLPTR;
        return f;
    }
    
    template <typename Signature, Signature* Func>
    static binded_function create()
    {
        struct handler {
            static result_type transfer(void* /*arg1*/) {
                return Func();
            }
        };
        
        binded_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_binded_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = MGBASE_NULLPTR;
        
        return f;
    }
    
    template <typename Signature, Signature* Func, typename Arg1>
    static binded_function create(Arg1* arg1)
    {
        struct handler {
            static result_type transfer(void* a1) {
                return Func(*static_cast<Arg1*>(a1));
            }
        };
        
        binded_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_binded_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = arg1;
        
        return f;
    }
    
    result_type operator() () {
        MGBASE_ASSERT(untyped_.func != MGBASE_NULLPTR);
        return reinterpret_cast<result_type (*)(void*)>(untyped_.func)(untyped_.arg1);
    }
    
    bool operator == (const binded_function& func) const MGBASE_NOEXCEPT {
        return detail::equal(untyped_, func.untyped_);
    }
    
private:
    mgbase_untyped_binded_function untyped_;
};

template <typename Result, typename Arg2>
class binded_function<Result (Arg2)>
{
    typedef Result  result_type;
    
public:
    template <typename Signature, Signature* Func, typename Arg1>
    static binded_function create(Arg1* arg1)
    {
        struct handler {
            static result_type transfer(void* a1, Arg2 arg2) {
                return Func(*static_cast<Arg1*>(a1), arg2);
            }
        };
        
        binded_function f;
        f.untyped_.func = reinterpret_cast<mgbase_untyped_binded_function_func_ptr_t>(handler::transfer);
        f.untyped_.arg1 = arg1;
        
        return f;
    }
    
    result_type operator() (Arg2 arg2) {
        MGBASE_ASSERT(untyped_.func != MGBASE_NULLPTR);
        return reinterpret_cast<result_type (*)(void*, Arg2)>(untyped_.func)(untyped_.arg1, arg2);
    }
    
    bool operator == (const binded_function& func) const MGBASE_NOEXCEPT {
        return detail::equal(untyped_, func.untyped_);
    }
    
private:
    untyped_binded_function untyped_;
};

namespace detail {

template <typename S>
struct binded_function_signature;

template <typename R>
struct binded_function_signature<R ()>
{
    typedef R (type)();
};

template <typename R, typename A1>
struct binded_function_signature<R (A1)>
{
    typedef R (type)();
};

template <typename R, typename A1, typename A2>
struct binded_function_signature<R (A1, A2)>
{
    typedef R (type)(A2);
};

} // namespace detail 

namespace /*unnamed*/ {

template <typename Signature, Signature* Func>
inline binded_function<typename detail::binded_function_signature<Signature>::type> make_binded_function() {
    return binded_function<typename detail::binded_function_signature<Signature>::type>::template create<Signature, Func>();
}

template <typename Signature, Signature* Func, typename Arg1>
inline binded_function<typename detail::binded_function_signature<Signature>::type> make_binded_function(Arg1* arg1) {
    return binded_function<typename detail::binded_function_signature<Signature>::type>::template create<Signature, Func>(arg1);
}

} // unnamed namespace

} // namespace mgbase

