
#pragma once

#include <menps/mecom/rpc/call.hpp>
#include <menps/mecom/rpc/requester.hpp>

namespace menps {
namespace mecom {
namespace rpc {

struct rpc_policy
{
    typedef handler_id_t    handler_id_type;
    
    template <typename T>
    struct client_request_message_type {
        typedef client_request_message<T>   type;
    };
    
    static client_request_message<void> allocate_request(mefdn::size_t alignment, mefdn::size_t size) {
        return client_request_message<void>::convert_from(
            detail::allocate_message(alignment, size)
        );
    }
    
    template <typename T, typename... Args>
    static client_request_message<T> make_request(Args&&... args)
    {
        return mecom::rpc::make_request<T>(
            mefdn::forward<Args>(args)...
        );
    }
    
    template <typename Handler>
    static void register_handler() {
        register_handler<Handler>(Handler{});
    }
    template <typename Handler>
    static void register_handler(Handler handler) {
        mecom::rpc::register_handler2<Handler>(
            mecom::rpc::requester::get_instance()
        ,   handler
        );
    }
    template <typename Handler>
    static void register_handler(requester& rqstr, Handler handler) {
        mecom::rpc::register_handler2<Handler>(
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
    ,   const mefdn::callback<void ()>                 on_complete
    ) {
        return mecom::rpc::call2_async<Handler>(
            rqstr
        ,   target_proc
        ,   mefdn::move(rqst_msg)
        ,   on_complete
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> async_call(
        requester&                              rqstr
    ,   const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ,   const mefdn::callback<void ()>         on_complete
    ) {
        return mecom::rpc::call2_async<Handler>(
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
        return mecom::rpc::call<Handler>(
            rqstr
        ,   target_proc
        ,   mefdn::move(rqst_msg)
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        requester&                              rqstr
    ,   const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        return mecom::rpc::call<Handler>(
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
        return mecom::rpc::call<Handler>(
            mecom::rpc::requester::get_instance()
        ,   target_proc
        ,   mefdn::move(rqst_msg)
        );
    }
    template <typename Handler>
    static client_reply_message<typename Handler::reply_type> call(
        const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        return mecom::rpc::call<Handler>(
            mecom::rpc::requester::get_instance()
        ,   target_proc
        ,   rqst_data
        );
    }
};

} // namespace rpc
} // namespace mecom
} // namespace menps

