
#pragma once

#include <mgcom/rpc/call2.hpp>

namespace mgcom {
namespace rpc {

struct rpc_policy
{
    typedef handler_id_t    handler_id_type;
    
    template <typename T>
    static request_message<T> make_request() {
        return mgcom::rpc::make_request<T>();
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
    static reply_message<typename Handler::reply_type> call(
        const process_id_t                              target_proc
    ,   request_message<typename Handler::request_type> rqst_msg
    ) {
        return mgcom::rpc::call2<Handler>(
            mgcom::rpc::requester::get_instance()
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        );
    }
    template <typename Handler>
    static reply_message<typename Handler::reply_type> call(
        const process_id_t                      target_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        return mgcom::rpc::call2<Handler>(
            mgcom::rpc::requester::get_instance()
        ,   target_proc
        ,   rqst_data
        );
    }
};

} // namespace rpc
} // namespace mgcom

