
#pragma once

#include "deferred.hpp"
#include "control_block.hpp"

namespace mgbase {

namespace detail {

template <
    typename    Signature
,   Signature   Func
,   typename    CB
>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
resumable make_deferred_pass(CB& cb)
{
    typedef typename mgbase::function_traits<Signature>::result_type    return_type;
    typedef typename detail::deferred_result<Signature>::type T;
    
    deferred<T> df = call_with_value_wrapper<return_type>(
        MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(Func)
    ,   wrap_reference(cb) // value_wrapper<CB&>
    );
    
    continuation<T>* const current_cont = df.get_continuation();
    continuation<T>& next_cont = get_next_continuation<T>(cb);
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return next_cont.call(df.to_ready());
    }
    else
    {
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

namespace /*unnamed*/ {

template <
    typename    Signature
,   Signature   Func
,   typename    CB
>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
deferred<typename detail::deferred_result<Signature>::type>
make_deferred(
    inlined_function<Signature, Func>   /*ignored*/
,   CB&                                 cb
) MGBASE_NOEXCEPT
{
    typedef typename detail::deferred_result<Signature>::type T;
    
    continuation<T>& cont = get_next_continuation<T>(cb);
    
    return deferred<T>(
        cont
    ,   make_resumable(
            MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(
                &detail::make_deferred_pass<Signature, Func, CB>
            )
        ,   cb
        )
    );
}

} // unnamed namespace

} // namespace mgbase

