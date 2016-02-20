
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/basic_active_object.hpp>
#include <mgbase/optional.hpp>
#include "basic_command.hpp"

namespace mgcom {

template <typename R, typename Func>
struct comm_call_cb
{
    mgbase::atomic<bool>        flag;
    mgbase::value_wrapper<R>    result;
    
    mgbase::continuation<R>     cont;
    Func                        func;
};

namespace detail {

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE void execute_comm_call(comm_call_cb<R, Func>& cb)
{
    cb.result = mgbase::call_with_value_wrapper<R>(cb.func, mgbase::wrap_void());
    
    cb.flag.store(true, mgbase::memory_order_release);
}

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE mgbase::deferred<R> comm_call_check(comm_call_cb<R, Func>& cb)
{
    if (cb.flag.load(mgbase::memory_order_acquire))
        return cb.result;
    else
        return mgbase::make_deferred<
            mgbase::deferred<R> (comm_call_cb<R, Func>&)
        ,   comm_call_check
        >
        (cb);
}

bool try_comm_call(const mgbase::callback_function<void ()>&);

} // namespace detail

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
mgbase::optional< mgbase::deferred<R> > try_comm_call_async(comm_call_cb<R, Func>& cb, const Func& func)
{
    cb.flag.store(false, mgbase::memory_order_relaxed);
    cb.func = func;
    
    const mgbase::callback_function<void ()> callback_func
        = mgbase::make_callback_function(
            mgbase::bind1st_of_1(
                MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&detail::execute_comm_call<R, Func>)
            ,   mgbase::wrap_reference(cb)
            )
        );
    
    const bool ret = detail::try_comm_call(callback_func);
    
    if (MGBASE_UNLIKELY(!ret))
        return mgbase::nullopt;
    
    return mgbase::make_deferred<
        mgbase::deferred<R> (comm_call_cb<R, Func>&)
    ,   &detail::comm_call_check<R>
    >
    (cb);
}

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
mgbase::deferred<R> comm_call_async(comm_call_cb<R, Func>& cb, const Func& func)
{
    while (true)
    {
        const mgbase::optional< mgbase::deferred<R> > ret
            = try_comm_call_async<R>(cb, func);
        
        if (ret)
            return *ret;
    }
}

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE R comm_call(const Func& func)
{
    comm_call_cb<R, Func> cb;
    return comm_call_async<R>(cb, func).wait();
}

} // namespace mgcom

