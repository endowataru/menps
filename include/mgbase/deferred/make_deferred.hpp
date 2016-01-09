
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
struct make_deferred_handler
{
    typedef typename detail::deferred_result<Signature>::type T;
    
    static MGBASE_ALWAYS_INLINE resumable transfer(CB& cb)
    {
        deferred<T> df = Func(cb);
        
        continuation<T>* const current_cont = df.get_continuation();
        
        if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
        {
            continuation<T>& next_cont = get_next_continuation<T>(cb);
            return next_cont.call(df.to_ready());
        }
        else {
            MGBASE_ASSERT(current_cont == &(get_next_continuation<T>(cb)));
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
>
inline MGBASE_ALWAYS_INLINE
deferred<typename detail::deferred_result<Signature>::type> make_deferred(CB& cb)
{
    typedef typename detail::deferred_result<Signature>::type T;
    
    return deferred<T>(
        get_next_continuation<T>(cb)
    ,   make_binded_function<
            resumable (CB&)
        ,   detail::make_deferred_handler<Signature, Func, CB>::transfer
        >
        (&cb)
    );
}

} // unnamed namespace

} // namespace mgbase

