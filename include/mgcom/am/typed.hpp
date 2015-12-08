
#pragma once

#include <mgcom/rma/untyped.hpp>

namespace mgcom {
namespace am {

template <typename Signature, Signature* Func>
class handler_traits {

};

/*

struct example_handler {
    static const mgcom::am::handler_id_t request_id;
    static const mgcom::am::handler_id_t reply_id;
    
    typedef int     request_argument;
    typedef double  reply_argument;
    
    static reply_argument on_request(const mgcom::am::callback_parameters& params, const request_argument& arg) {
        
    }
    
    static void on_reply(const mgcom::am::callback_parameters& params, const reply_argument& arg) {
        
    }
};

*/

template <typename Handler>
inline void call_roundtrip_nb(
    send_cb&                            cb
,   process_id_t                        proc_id
,   typename Handler::request_argument& arg
)
{
    untyped::send_nb(
        cb
    ,   Handler::request_id
    ,   &arg
    ,   sizeof(arg)
    ,   proc_id
    );
}

template <typename Handler>
inline void register_roundtrip_handler()
{
    struct transfer
    {
        static void request(const callback_parameters* params)
        {
            const typename Handler::request_argument& arg
                = *static_cast<const typename Handler::request_argument*>(
                    params->data
                );
            
            typename Handler::reply_argument result
                = Handler::on_request(*params, arg);
            
            untyped::reply(
                params
            ,   Handler::reply_id
            ,   &result
            ,   sizeof(typename Handler::reply_argument)
            );
        }
        
        static void reply(const callback_parameters* params)
        {
            const typename Handler::reply_argument& arg
                = *static_cast<const typename Handler::reply_argument*>(
                    params->data
                );
            
            Handler::on_reply(*params, arg);
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

