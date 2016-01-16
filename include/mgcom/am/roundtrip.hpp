
#pragma once

#include <mgcom/rma/untyped.hpp>
#include "roundtrip.h"
#include <mgbase/threading/atomic.hpp>

namespace mgcom {
namespace am {

/*

struct example_handler {
    static const mgcom::am::handler_id_t request_id;
    static const mgcom::am::handler_id_t reply_id;
    
    typedef int     request_argument;
    typedef double  reply_argument;
    
    static reply_argument on_request(const mgcom::am::callback_parameters& params, const request_argument& arg) {
        
    }
};

*/

typedef mgcom_am_call_roundtrip_cb  call_roundtrip_cb;

namespace detail {

template <typename Handler>
struct call_roundtrip_types {
    typedef typename Handler::request_argument  request_argument;
    typedef typename Handler::reply_argument    reply_argument;
    
    struct request_info {
        call_roundtrip_cb*  cb;
        request_argument    arg;
    };
    
    struct reply_info {
        call_roundtrip_cb*  cb;
        reply_argument      arg;
    };
    
    typedef mgbase::deferred<void>              result_type;
};

template <typename Handler>
class call_roundtrip_handlers
{
    typedef call_roundtrip_types<Handler>       types;
    
    typedef call_roundtrip_cb                   cb_type;
    typedef typename types::result_type         result_type;
    
    typedef typename types::request_argument    request_argument;
    typedef typename types::reply_argument      reply_argument;
    
    typedef typename types::request_info        request_info;
    typedef typename types::reply_info          reply_info;
    
public:
    static result_type start(
        cb_type&                    cb
    ,   process_id_t                proc_id
    ,   const request_argument&     arg
    ,   reply_argument*             result
    )
    {
        // This flag is important to tell the finalization.
        cb.got_reply = false;
        cb.result    = result; // implicitly casted to void*
        
        request_info info = { &cb, arg };
        
        return mgbase::add_continuation<result_type (cb_type&), check>(
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
            
            return mgbase::make_deferred<result_type (cb_type&), check>(cb);
        }
    }
};

} // namespace detail


template <typename Handler>
inline typename detail::call_roundtrip_types<Handler>::result_type
call_roundtrip_nb(
    call_roundtrip_cb&                                                      cb
,   process_id_t                                                            proc_id
,   const typename detail::call_roundtrip_types<Handler>::request_argument& arg
,   typename detail::call_roundtrip_types<Handler>::reply_argument*         result
)
{
    return detail::call_roundtrip_handlers<Handler>::start(cb, proc_id, arg, result);
}

template <typename Handler>
inline void register_roundtrip_handler()
{
    typedef detail::call_roundtrip_types<Handler>   types;
    
    typedef typename types::reply_argument          reply_argument;
    
    typedef typename types::request_info            request_info;
    typedef typename types::reply_info              reply_info;
    
    struct transfer
    {
        static void request(const callback_parameters* params)
        {
            const request_info& req_info =
                *static_cast<const request_info*>(params->data);
            
            const reply_argument result = Handler::on_request(*params, req_info.arg);
            const reply_info rep_info = { req_info.cb, result };
            
            untyped::reply(
                params
            ,   Handler::reply_id
            ,   &rep_info
            ,   sizeof(rep_info)
            );
        }
        
        static void reply(const callback_parameters* params)
        {
            const reply_info& rep_info =
                *static_cast<const reply_info*>(params->data);
            
            reply_argument* dest = static_cast<reply_argument*>(rep_info.cb->result);
            
            // Copy the result to the specified destination.
            *dest = rep_info.arg;
            
            // Set the flag to finalize.
            mgbase::atomic_store_explicit(&rep_info.cb->got_reply, true, mgbase::memory_order_release);
        }
    };
    
    untyped::register_handler(
        Handler::request_id
    ,   &transfer::request
    );
    untyped::register_handler(
        Handler::reply_id
    ,   &transfer::reply
    );
}

} // namespace am
} // namespace mgcom

