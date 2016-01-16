
#pragma once

#include <mgcom/rma/untyped.hpp>
#include "roundtrip.h"
#include <mgbase/threading/atomic.hpp>
#include <mgbase/type_traits.hpp>

namespace mgcom {
namespace am {

/*

struct example_handler {
    static const mgcom::am::handler_id_t request_id;
    static const mgcom::am::handler_id_t reply_id;
    
    typedef int     argument_type;
    typedef double  return_type;
    
    static return_type on_request(const mgcom::am::callback_parameters& params, const argument_type& arg) {
        
    }
};

*/

typedef mgcom_am_call_roundtrip_cb  call_roundtrip_cb;

namespace detail {

template <typename Handler>
struct call_roundtrip_types
{
    typedef typename Handler::argument_type argument_type;
    typedef typename Handler::return_type   return_type;
    
    struct argument_info
        : mgbase::value_wrapper<argument_type>
    {
        argument_info(call_roundtrip_cb* cb_ptr, const mgbase::value_wrapper<argument_type>& val)
            : mgbase::value_wrapper<argument_type>(val)
            , cb(cb_ptr)
            { }
        
        call_roundtrip_cb*  cb;
    };
    
    struct return_info
        : mgbase::value_wrapper<return_type>
    {
        return_info(call_roundtrip_cb* cb_ptr, const mgbase::value_wrapper<return_type>& val)
            : mgbase::value_wrapper<return_type>(val)
            , cb(cb_ptr)
            { }
        
        call_roundtrip_cb*  cb;
    };
    
    typedef mgbase::deferred<void>  result_type;
};

template <typename Handler>
class call_roundtrip_handlers
{
    typedef call_roundtrip_handlers             handlers_type;
    typedef call_roundtrip_types<Handler>       types;
    
    typedef call_roundtrip_cb                   cb_type;
    typedef typename types::result_type         result_type;
    
    typedef typename types::argument_type       argument_type;
    typedef typename types::return_type         return_type;
    
    typedef typename types::argument_info       argument_info;
    typedef typename types::return_info         return_info;
    
public:
    static result_type start(
        cb_type&                cb
    ,   process_id_t            proc_id
    ,   const argument_type&    arg
    )
    {
        // This flag is important to tell the finalization.
        cb.got_reply = false;
        
        argument_info info(&cb, arg);
        
        return mgbase::add_continuation<result_type (cb_type&), &handlers_type::check>(
            cb
        ,   untyped::send_nb(
                cb.cb_send
            ,   Handler::request_id
            ,   &info
            ,   sizeof(info)
            ,   proc_id
            )
        );
    }
    
private:
    static result_type check(cb_type& cb)
    {
        if (mgbase::atomic_load_explicit(&cb.got_reply, mgbase::memory_order_acquire)) {
            return mgbase::make_ready_deferred();
        }
        else {
            // TODO: Selective polling
            mgcom::am::poll();
            
            return mgbase::make_deferred<result_type (cb_type&), &handlers_type::check>(cb);
        }
    }
};

} // namespace detail

template <typename Handler>
inline typename detail::call_roundtrip_types<Handler>::result_type
call_roundtrip_nb(
    call_roundtrip_cb&                                                      cb
,   process_id_t                                                            proc_id
,   const typename detail::call_roundtrip_types<Handler>::argument_type&    arg
,   typename detail::call_roundtrip_types<Handler>::return_type*            result
) {
    cb.result = result; // implicitly casted to void*
    
    return detail::call_roundtrip_handlers<Handler>::start(cb, proc_id, arg);
}

template <typename Handler>
inline typename detail::call_roundtrip_types<Handler>::result_type
call_roundtrip_nb(
    call_roundtrip_cb&                                                      cb
,   process_id_t                                                            proc_id
,   const typename detail::call_roundtrip_types<Handler>::argument_type&    arg
) {
    MGBASE_STATIC_ASSERT(
        mgbase::is_void<
            typename detail::call_roundtrip_types<Handler>::return_type
        >::value
    ,   "return_type must be void"
    );
    
    return detail::call_roundtrip_handlers<Handler>::start(cb, proc_id, arg);
}

namespace detail {

namespace /*unnamed*/ {

template <typename Handler>
inline void roundtrip_request(const callback_parameters* params)
{
    typedef call_roundtrip_types<Handler>       types;
    
    typedef typename types::argument_info       argument_info;
    typedef typename types::argument_type       argument_type;
    
    typedef typename types::return_type         return_type;
    typedef typename types::return_info         return_info;
    
    const argument_info& arg_info =
        *static_cast<const argument_info*>(params->data);
    
    const mgbase::value_wrapper<return_type> res = 
        mgbase::call_by_value_wrapper<return_type>(
            mgbase::make_bound_function<
                return_type (const mgcom::am::callback_parameters&, const argument_type&)
            ,   &Handler::on_request
            >(params)
        ,   arg_info
        );
    
    return_info ret_info(arg_info.cb, res);
    
    untyped::reply(
        params
    ,   Handler::reply_id
    ,   &ret_info
    ,   sizeof(ret_info)
    );
}

template <typename Handler>
inline void roundtrip_reply(const callback_parameters* params)
{
    typedef call_roundtrip_types<Handler>       types;
    
    typedef typename types::return_type         return_type;
    typedef typename types::return_info         return_info;
    
    const return_info& ret_info =
        *static_cast<const return_info*>(params->data);
    
    return_type* dest = static_cast<return_type*>(ret_info.cb->result);
    
    // Copy the result to the specified destination.
    ret_info.assign_to(dest);
    
    // Set the flag to finalize.
    mgbase::atomic_store_explicit(&ret_info.cb->got_reply, true, mgbase::memory_order_release);
}

} // unnamed namespace

} // namespace detail

template <typename Handler>
inline void register_roundtrip_handler()
{
    untyped::register_handler(
        Handler::request_id
    ,   &detail::roundtrip_request<Handler>
    );
    untyped::register_handler(
        Handler::reply_id
    ,   &detail::roundtrip_reply<Handler>
    );
}

} // namespace am
} // namespace mgcom

