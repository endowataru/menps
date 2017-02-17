
#pragma once

#include <mgcom/rpc/call.hpp>
#include <mgcom/rpc/requester.hpp>

namespace mgcom {
namespace rpc {

struct rpc_policy
{
    typedef handler_id_t    handler_id_type;
    
    template <typename T, typename... Args>
    static client_request_message<T> make_request(Args&&... args)
    {
        return mgcom::rpc::make_request<T>(
            mgbase::forward<Args>(args)...
        );
    }
    
    template <typename Handler>
    static void register_handler() {
        register_handler<Handler>(Handler{});
    }
    template <typename Handler>
    static void register_handler(Handler handler) {
        mgcom::rpc::register_handler2<Handler>(
            mgcom::rpc::requester::get_instance()
        ,   handler
        );
    }
    template <typename Handler>
    static void register_handler(requester& rqstr, Handler handler) {
        mgcom::rpc::register_handler2<Handler>(
            rqstr
        ,   handler
        );
    }
    
    #if 0
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> async_call(
        requester&                                      rqstr
    ,   const process_id_t                              target_proc
    ,   client_request_message<typename Handler::request_type> rqst_msg
    ,   const mgbase::callback<void ()>                 on_complete
    ) {
        return mgcom::rpc::call2_async<Handler>(
            rqstr
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        ,   on_complete
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> async_call(
        requester&                              rqstr
    ,   const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ,   const mgbase::callback<void ()>         on_complete
    ) {
        return mgcom::rpc::call2_async<Handler>(
            rqstr
        ,   target_proc
        ,   make_request<typename Handler::request_type>(rqst_data)
        ,   on_complete
        );
    }
    #endif
    
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        requester&                                              rqstr
    ,   const process_id_t                                      target_proc
    ,   client_request_message<typename Handler::request_type>  rqst_msg
    ) {
        return mgcom::rpc::call<Handler>(
            rqstr
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        requester&                              rqstr
    ,   const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        return mgcom::rpc::call<Handler>(
            rqstr
        ,   target_proc
        ,   rqst_data
        );
    }
    
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        const process_id_t                                      target_proc
    ,   client_request_message<typename Handler::request_type>  rqst_msg
    ) {
        return mgcom::rpc::call<Handler>(
            mgcom::rpc::requester::get_instance()
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        return mgcom::rpc::call<Handler>(
            mgcom::rpc::requester::get_instance()
        ,   target_proc
        ,   rqst_data
        );
    }
};

} // namespace rpc
} // namespace mgcom

