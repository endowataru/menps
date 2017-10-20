
#pragma once

#include "delegator.hpp"
#include <menps/mefdn/callback.hpp>
#include <menps/mecom/ult.hpp>

namespace menps {
namespace mecom {

namespace detail {

template <typename Func>
class delegation
    : private Func
{
public:
    /*implicit*/ delegation(const Func& func)
        : Func(func) { }
    
    static void copy_to(const Func& src, void* const dest)
    {
        MEFDN_LOG_DEBUG("msg:Set delegated params.");
        
        // Construct a closure at the specified location.
        *static_cast<Func*>(dest) = src;
    }
    
    static bool execute(const void* const ptr)
    {
        auto& f = *static_cast<const Func*>(ptr);
        return f();
    }
};

} // namespace detail

template <typename Func>
inline bool try_delegate(delegator& del, const Func& func)
{
    typedef detail::delegation<Func> del_type;
    
    return del.try_execute({
        {
            [&] (void* const dest) {
                del_type::copy_to(func, dest);
            }
        }
        #if 0
        mefdn::make_callback_function(
            mefdn::bind1st_of_2(
                MEFDN_MAKE_INLINED_FUNCTION_TEMPLATE(del_type::copy_to)
            ,   mefdn::wrap_reference(func)
            )
        )
        #endif
    ,   &del_type::execute
    });
}

template <typename Func>
inline void delegate(delegator& del, const Func& func)
{
    while (MEFDN_UNLIKELY(
        ! try_delegate(del, func)
    )) {
        ult::this_thread::yield();
    }
}


namespace detail {

template <typename Func>
class execution
    : private Func
{
public:
    execution(const Func& func, const mefdn::callback<void ()>& on_comp)
        : Func(func)
        , on_complete(on_comp) { }
    
    execution(const execution&) = default;
    execution& operator = (const execution&) = default;
    
    bool operator() () const
    {
        auto& f = *static_cast<const Func*>(this);
        const bool ret = f();
        
        if (MEFDN_LIKELY(ret)) {
            // Execute the callback.
            on_complete();
        }
        return ret;
    }
    
private:
    mefdn::callback<void ()> on_complete;
};

} // namespace detail


template <typename Func>
inline bool try_execute_async(delegator& del, const Func& func, const mefdn::callback<void ()>& on_complete)
{
    return try_delegate(
        del
    ,   detail::execution<Func>{ func, on_complete }
    );
}

template <typename Func>
inline void execute(delegator& del, const Func& func)
{
    ult::sync_flag flag;
    
    while (MEFDN_UNLIKELY(
        ! try_execute_async(del, func, mefdn::make_callback_notify(&flag))
    )) {
        ult::this_thread::yield();
    }
    
    flag.wait();
}

} // namespace mecom
} // namespace menps

