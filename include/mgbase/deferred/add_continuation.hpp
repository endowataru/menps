
#pragma once

#include "call_and_defer.hpp"
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
,   typename T
>
struct add_continuation_handler
{
    typedef typename detail::deferred_result<Signature>::type U;
    
    static MGBASE_ALWAYS_INLINE resumable transfer(CB& cb, const ready_deferred<T>& val)
    {
        deferred<U> df = call_and_defer<U>(
            mgbase::make_bound_function<Signature, Func>(&cb)
        ,   val
        );
        
        continuation<U>* const current_cont = df.get_continuation();
        continuation<U>& next_cont = get_next_continuation<U>(cb);
        
        if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
        {
            return next_cont.call(df.to_ready());
        }
        else {
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
};

#ifdef MGBASE_IF_CPP11_SUPPORTED
} /* unnamed namespace */
#endif

} // namespace detail

namespace /*unnamed*/ {

template <
    typename Signature
,   Signature* Func
,   typename CB
,   typename T
>
inline MGBASE_ALWAYS_INLINE
deferred<typename detail::deferred_result<Signature>::type> add_continuation(CB& cb, deferred<T> df)
{
    typedef typename detail::deferred_result<Signature>::type U;
    
    continuation<T>* const current_cont = df.get_continuation();
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return call_and_defer<U>(
            mgbase::make_bound_function<Signature, Func>(&cb)
        ,   df.to_ready()
        );
    }
    else
    {
        current_cont->set(
            make_bound_function<
                resumable (CB&, const ready_deferred<T>&)
            ,   &detail::add_continuation_handler<Signature, Func, CB, T>::transfer
            >
            (&cb)
        );
        
        return deferred<U>(get_next_continuation<U>(cb), df.get_resumable());
    }
}

} /* unnamed namespace */

} // namespace mgbase

