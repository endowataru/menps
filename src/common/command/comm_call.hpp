
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/deferred.hpp>
#include <mgbase/optional.hpp>
#include <mgbase/logger.hpp>
#include "basic_command.hpp"

namespace mgcom {

template <typename R, typename Func>
struct comm_call_cb
{
    mgbase::continuation<R>     cont;
    mgbase::wait_flag           flag;
    
    mgbase::value_wrapper<R>    result;
    Func                        func;
};

namespace detail {

// Defined in another translation unit.
MGBASE_WARN_UNUSED_RESULT
bool try_comm_call(const mgbase::callback_function<void ()>&);

template <typename R, typename Func>
class comm_call_handlers
{
    typedef comm_call_cb<R, Func>   cb_type;
    typedef mgbase::deferred<R>     result_type;
    
public:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type start(cb_type& cb)
    {
        return mgbase::try_call_and_wait<config>(cb);
    }
    
private:
    struct config
    {
        typedef comm_call_handlers::cb_type         cb_type;
        typedef comm_call_handlers::result_type     result_type;
        
        MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
        static bool try_call(cb_type& cb)
        {
            const mgbase::callback_function<void ()> f
                = mgbase::make_callback_function(
                    mgbase::bind1st_of_1(
                        MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(execute)
                    ,   mgbase::wrap_reference(cb)
                    )
                );
            
            return try_comm_call(f);
        }
        
        MGBASE_ALWAYS_INLINE
        static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
            return cb.flag;
        }
        
        MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
        static result_type on_complete(cb_type& cb) MGBASE_NOEXCEPT {
            return cb.result;
        }
    };
    
    MGBASE_ALWAYS_INLINE
    static void execute(cb_type& cb)
    {
        cb.result = mgbase::call_with_value_wrapper<R>(cb.func, mgbase::wrap_void());
        
        mgbase::set_notfiied(&cb.flag);
    }
};

} // namespace detail

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
mgbase::deferred<R> comm_call_async(comm_call_cb<R, Func>& cb, const Func& func)
{
    cb.func = func;
    
    return detail::comm_call_handlers<R, Func>::start(cb);
}

template <typename R, typename Func>
MGBASE_ALWAYS_INLINE
R comm_call(const Func& func)
{
    comm_call_cb<R, Func> cb;
    return comm_call_async<R>(cb, func).wait();
}

} // namespace mgcom

