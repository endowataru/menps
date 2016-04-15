
#pragma once

#include "deferred.hpp"
#include "control_block.hpp"

namespace mgbase {

namespace detail {

template <
    typename    Signature
,   Signature   Func
,   typename    CB
,   typename    T
>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
resumable add_continuation_pass(CB& cb, const value_wrapper<T>& val)
{
    typedef typename detail::deferred_result<Signature>::type           U;
    typedef typename mgbase::function_traits<Signature>::result_type    return_type;
    
    // Call the current continuation function.
    
    deferred<U> df = call_with_value_wrapper_2<return_type>(
        MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(Func)
    ,   mgbase::wrap_reference(cb)
    ,   val
    );
    
    continuation<U>* const current_cont = df.get_continuation();
    continuation<U>& next_cont = get_next_continuation<U>(cb);
    
    // Check whether the returned value is ready or not.
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        // Immediately call the next continuation function.
        
        return next_cont.call(df.to_ready());
    }
    else {
        // The next continuation cannot be executed now.
        
        if (current_cont != &next_cont) {
            // Move the continuation.
            *current_cont = next_cont;
        }
        else {
            // The continuation has already been set within Func.
        }
        
        return df.get_resumable();
    }
}

} // namespace detail

template <typename T>
template <
    typename    Signature
,   Signature   Func
,   typename    CB
>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
deferred<typename detail::deferred_result<Signature>::type>
deferred<T>::add_continuation(
    inlined_function<Signature, Func>   /*ignored*/
,   CB&                                 cb
) {
    continuation<T>* const current_cont = this->get_continuation();
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return call_with_value_wrapper_2<typename mgbase::function_traits<Signature>::result_type>(
            MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(Func)
        ,   mgbase::wrap_reference(cb)
        ,   this->to_ready()
        );
    }
    else
    {
        current_cont->set(
            mgbase::make_callback_function(
                mgbase::bind1st_of_2(
                    MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(
                        &detail::add_continuation_pass<Signature, Func, CB, T>
                    )
                ,   mgbase::wrap_reference(cb)
                )
            )
        );
        
        typedef typename detail::deferred_result<Signature>::type   next_return_type;
        
        return deferred<next_return_type>(
            get_next_continuation<next_return_type>(cb)
        ,   this->get_resumable()
        );
    }
}

} // namespace mgbase

