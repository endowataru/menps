
#pragma once

#include <menps/mecom2/rpc/server_context.hpp>

namespace menps {
namespace mecom2 {

template <typename P, typename Handler>
struct rpc_server_context_policy
{
    using derived_type = server_context<rpc_server_context_policy>;
    using server_type = typename P::server_type;
    
    using process_id_type = typename P::process_id_type;
    
    using server_request_message_type =
        typename P::template server_request_message<
            typename Handler::request_type
        >;
    
    using server_reply_message_type =
        typename P::template server_reply_message<
            typename Handler::reply_type
        >;
};

template <typename P, typename Handler>
class rpc_handler_callback
{
    using request_type = typename Handler::request_type;
    using reply_type = typename Handler::reply_type;
    
    using server_reply_message_type = typename P::template server_reply_message<void>;
    
public:
    template <typename... Args>
    /*implicit*/ rpc_handler_callback(Args&&... args)
        : h_(mefdn::forward<Args>(args)...)
    { }
    
    template <typename HandlerContext>
    server_reply_message_type operator() (HandlerContext&& hc) const
    {
        server_context< rpc_server_context_policy<P, Handler> > sc{
            hc.server
        ,   hc.src_proc
        ,   mefdn::move(hc.rqst_msg)
                .template reinterpret_cast_to<request_type>()
        };
        
        auto ret_msg = this->h_(sc);
        
        return mefdn::move(ret_msg)
            .template reinterpret_cast_to<void>();
    }
    
private:
    Handler h_;
};

template <typename P>
class rpc_typed_handler
{
    MEFDN_DEFINE_DERIVED(P)
    
    using process_id_type = typename P::process_id_type;
    
public:
    template <typename Handler>
    void add_handler(Handler&& h)
    {
        auto& self = this->derived();
        auto& inv = self.get_invoker();
        
        using handler_type = mefdn::decay_t<Handler>;
        
        inv.add_handler({
            handler_type::handler_id
        ,   rpc_handler_callback<P, handler_type>(
                mefdn::forward<Handler>(h)
            )
        });
    }
    
    template <typename Handler>
    typename P::template client_reply_message< typename Handler::reply_type >
    call(
        const process_id_type server_proc
    ,   typename P::template client_request_message< typename Handler::request_type >
            rqst_msg
    ) {
        using reply_type = typename Handler::reply_type;
        auto& self = this->derived();
        
        // Note: Braced-initializer-list is disabled for GCC 4.8.
        auto ret = self.untyped_call(/*{*/
            server_proc
        ,   Handler::handler_id
        ,   mefdn::move(rqst_msg)
                .template reinterpret_cast_to<void>()
        /*}*/);
        
        return mefdn::move(ret)
            .template reinterpret_cast_to<reply_type>();
    }
    
    template <typename Handler>
    typename P::template client_reply_message< typename Handler::reply_type >
    call(
        const process_id_type                   server_proc
    ,   const typename Handler::request_type&   rqst_data
    ) {
        auto& self = this->derived();
        auto& client = self.get_client();
        
        using request_type = typename Handler::request_type;
        
        // Allocate a buffer.
        // TODO: Skip this allocation if unnecessary.
        auto req = client.allocate_client_request( sizeof(request_type) );
        
        // Copy the data to the buffer.
        // TODO: Skip this copy if unnecessary.
        new (req.get()) request_type(rqst_data);
        
        return this->template call<Handler>(
            server_proc
        ,   mefdn::move(req)
                .template reinterpret_cast_to<request_type>()
        );
    }
};

} // namespace mecom2
} // namespace menps

