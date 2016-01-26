
#pragma once

#include "deferred.hpp"
#include "control_block.hpp"

namespace mgbase {

namespace detail {

#ifdef MGBASE_IF_CPP11_SUPPORTED
namespace /*unnamed*/ {
#endif

template <
    typename Signature
,   Signature* Func
,   typename CB
>
MGBASE_ALWAYS_INLINE resumable make_deferred_pass(CB& cb)
{
    typedef typename mgbase::function_traits<Signature>::result_type    return_type;
    typedef typename detail::deferred_result<Signature>::type T;
    
    deferred<T> df = call_with_value_wrapper<return_type>(
        Func
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

#ifdef MGBASE_IF_CPP11_SUPPORTED
} /* unnamed namespace */
#endif

} // namespace detail

namespace /*unnamed*/ {

template <
    typename Signature
,   Signature* Func
,   typename CB
>
MGBASE_ALWAYS_INLINE deferred<typename detail::deferred_result<Signature>::type>
make_deferred(CB& cb)
{
    typedef typename detail::deferred_result<Signature>::type T;
    
    return deferred<T>(
        get_next_continuation<T>(cb)
    ,   make_resumable(
            make_bound_function<
                resumable (CB&)
            ,   &detail::make_deferred_pass<Signature, Func, CB>
            >
            (&cb)
        )
    );
}

} // unnamed namespace

} // namespace mgbase

