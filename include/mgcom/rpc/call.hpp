
#pragma once

#include <mgcom/rpc/client.hpp>
#include <mgcom/rpc/server.hpp>
#include <mgcom/rpc/message.hpp>
#include <mgcom/ult.hpp>
#include <mgbase/type_traits/is_void.hpp>

namespace mgcom {
namespace rpc {

/*
    struct example_fixed_sized_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = ???;
        
        typedef double  request_type;
        typedef int     reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& req = sc.request();
            const auto size = sc.size_in_bytes();
            const auto src_proc = sc.src_proc();
            
            // ...
            
            
            auto ret = server.make_reply();
            // *ret = ...;
            return ret;
        }
    };
    
    struct example_variable_length_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = ???;
        
        typedef double  request_type;
        typedef int     reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& req = sc.request();
            const auto size = sc.size_in_bytes();
            const auto src_proc = sc.src_proc();
            
            // ...
            
            
            auto ret =
                sc.allocate_reply(
                    MGBASE_ALIGNOF(reply_type)
                ,   sizeof(reply_type) + // ...
                );
            
            // *ret = ...;
            
            return ret;
        }
    };
*/

namespace detail {

template <typename Handler>
class server_context
{
    typedef typename Handler::request_type  request_type;
    typedef typename Handler::reply_type    reply_type;
    
public:
    typedef server_reply_message<reply_type>  return_type;
    
    server_context(
        const process_id_t                      src_proc
    ,   server_request_message<request_type>    rqst_msg
    )
        : src_proc_(src_proc)
        , rqst_msg_(mgbase::move(rqst_msg))
    { }
    
    // getter functions
    
    const request_type& request() const MGBASE_NOEXCEPT {
        return *this->request_ptr();
    }
    const request_type* request_ptr() const MGBASE_NOEXCEPT {
        return this->rqst_msg_.get();
    }
    
    mgbase::size_t size_in_bytes() const MGBASE_NOEXCEPT {
        return this->rqst_msg_.size_in_bytes();
    }
    
    process_id_t src_proc() const MGBASE_NOEXCEPT {
        return this->src_proc_;
    }
    
    // helper functions
    
    template <typename... Args>
    return_type make_reply(Args&&... args) const {
        return return_type::convert_from(
            detail::make_message<reply_type>(mgbase::forward<Args>(args)...)
        );
    }
    
private:
    process_id_t                            src_proc_;
    server_request_message<request_type>    rqst_msg_;
};

template <typename Handler>
struct handler_pass
{
    Handler h;
    
    untyped::handler_result operator() (untyped::handler_context_t hc) const
    {
        typedef typename Handler::request_type request_type;
        
        auto rqst_msg =
            server_request_message<request_type>::convert_from(
                mgbase::move(hc.rqst_msg)
            );
        
        server_context<Handler> sc{
            hc.src_proc
        ,   mgbase::move(rqst_msg)
        };
        
        auto ret_msg = (this->h)(sc);
        
        return untyped::handler_result{
            server_reply_message<void>::convert_from(
                mgbase::move(ret_msg)
            )
        };
    }
};

} // namespace detail

template <typename Handler>
void register_handler2(server& sv, Handler h)
{
    sv.add_handler({
        Handler::handler_id
    ,   detail::handler_pass<Handler>{ h }
    });
}


namespace detail {

template <typename Handler, typename Func>
struct call_complete_pass
{
    Func func;
    
    void operator() (client_reply_message<void>&& msg) const
    {
        typedef typename Handler::reply_type    reply_type;
        
        func(
            client_reply_message<reply_type>::convert_from(
                mgbase::move(msg)
            )
        );
    }
};

} // namespace detail

template <typename Handler, typename OnComplete>
inline ult::async_status<client_reply_message<typename Handler::reply_type>>
async_call(
    client&                                                 cli
,   const process_id_t                                      server_proc
,   client_request_message<typename Handler::request_type>  rqst_msg
,   OnComplete                                              on_complete
) {
    auto untyped_rqst_msg =
        client_request_message<void>::convert_from(
            mgbase::move(rqst_msg)
        );
    
    auto r =
        cli.async_call(untyped::async_call_params{
            server_proc
        ,   Handler::handler_id
        ,   mgbase::move(untyped_rqst_msg)
        ,   detail::call_complete_pass<Handler, OnComplete>{ on_complete }
        });
    
    // TODO: a bit ugly...
    if (r.is_ready()) {
        return ult::make_async_ready(
            client_reply_message<typename Handler::reply_type>::convert_from(
                mgbase::move(r.get())
            )
        );
    }
    else {
        return ult::make_async_deferred<
            client_reply_message<typename Handler::reply_type>
        >();
    }
}

namespace detail {

template <typename Handler>
struct async_call_functor
{
    requester&                                              rqstr;
    const process_id_t                                      target_proc;
    client_request_message<typename Handler::request_type>  rqst_msg;
    
    template <typename Cont>
    MGBASE_WARN_UNUSED_RESULT
    ult::async_status<client_reply_message<typename Handler::reply_type>>
    operator() (Cont& cont)
    {
        return async_call<Handler>(
            rqstr
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        ,   cont
        );
    }
};

} // namespace detail

template <typename Handler>
inline client_reply_message<typename Handler::reply_type> call(
    requester&                                              rqstr
,   const process_id_t                                      target_proc
,   client_request_message<typename Handler::request_type>  rqst_msg
) {
    typedef client_reply_message<typename Handler::reply_type>  reply_msg_type;
    
    return ult::suspend_and_call<reply_msg_type>(
        detail::async_call_functor<Handler>{
            rqstr
        ,   target_proc
        ,   mgbase::move(rqst_msg)
        }
    );
}

template <typename Handler>
inline client_reply_message<typename Handler::reply_type> call(
    requester&                              rqstr
,   const process_id_t                      target_proc
,   const typename Handler::request_type&   rqst_data
) {
    return call<Handler>(
        rqstr
    ,   target_proc
    ,   client_request_message<typename Handler::request_type>::convert_from(
            detail::make_message<typename Handler::request_type>(rqst_data)
        )
    );
}

} // namespace rpc
} // namespace mgcom

