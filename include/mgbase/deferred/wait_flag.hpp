
#pragma once

#include "deferred.hpp"
#include "wait_flag.h"
#include <mgbase/atomic.hpp>
#include <mgbase/callback.hpp>

namespace mgbase {

typedef mgbase_wait_flag    wait_flag;

MGBASE_ALWAYS_INLINE
void initialize_flag(wait_flag* const wf) MGBASE_NOEXCEPT
{
    wf->flag.store(mgbase::memory_order_relaxed);
}

MGBASE_ALWAYS_INLINE
callback<void ()> make_notification(wait_flag* const wf) MGBASE_NOEXCEPT
{
    MGBASE_ASSERT(!wf->flag.load());
    
    return mgbase::make_callback_store_release(&wf->flag, MGBASE_NONTYPE(true));
    //return mgbase::make_operation_store_release(&wf->flag, true);
}

MGBASE_ALWAYS_INLINE
void set_notfiied(wait_flag* const wf) MGBASE_NOEXCEPT
{
    (make_notification(wf))();
}

MGBASE_ALWAYS_INLINE
bool is_notified(const wait_flag& wf) MGBASE_NOEXCEPT
{
    return wf.flag.load(mgbase::memory_order_acquire);
}

namespace detail {

template <typename Config>
class wait_handlers
{
    typedef wait_handlers                   handlers;
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    
public:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type start(cb_type& cb)
    {
        wait_flag& wr = Config::get_flag(cb);
        initialize_flag(&wr);
        
        return try_(cb);
    }

private:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type try_(cb_type& cb)
    {
        if (MGBASE_LIKELY(Config::try_call(cb)))
            return wait(cb);
        else
            return mgbase::make_deferred(MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&handlers::try_), cb);
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type wait(cb_type& cb)
    {
        wait_flag& wr = Config::get_flag(cb);
        
        if (MGBASE_LIKELY(is_notified(wr)))
            return Config::on_complete(cb);
        else
            return mgbase::make_deferred(MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&handlers::wait), cb);
    }
};

} // namespace detail

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type try_call_and_wait(typename Config::cb_type& cb)
{
    return detail::wait_handlers<Config>::start(cb);
}

} // namespace mgbase


