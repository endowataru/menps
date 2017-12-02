
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/callback.hpp>
#include <menps/mefdn/vector.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rpc_invoker
{
    using process_id_type = typename P::process_id_type;
    
    using handler_id_type = typename P::handler_id_type;
    
    using server_type = typename P::server_type;
    using server_request_message_type = typename P::template server_request_message<void>;
    using server_reply_message_type = typename P::template server_reply_message<void>;
    
public:
    struct handler_context {
        server_type&                    server;
        process_id_type                 src_proc;
        server_request_message_type&&   rqst_msg;
    };
    
    using handler_context_type = const handler_context&;
    
    using handler_result_type = server_reply_message_type;
    
    using handler_callback_type =
        mefdn::callback<handler_result_type (handler_context_type)>;
    
    template <typename Conf>
    explicit rpc_invoker(Conf&& conf)
        : hs_(conf.max_num_handlers)
    { }
    
    struct add_handler_params
    {
        handler_id_type         id;
        handler_callback_type   cb;
    };
    
    void add_handler(const add_handler_params& p)
    {
        MEFDN_ASSERT(p.id < this->hs_.size());
        
        hs_[p.id] = p.cb;
    }
    
    handler_result_type call(
        server_type&                server
    ,   const handler_id_type       id
    ,   const process_id_type       src_proc
    ,   server_request_message_type rqst_msg
    ) {
        const auto& h = hs_[id];
        MEFDN_ASSERT(h);
        
        const handler_context_type hc{
            server
        ,   src_proc
        ,   mefdn::move(rqst_msg)
        };
        
        auto r = h(hc);
        
        return r;
    }
    
private:
    mefdn::vector<handler_callback_type> hs_;
};

} // namespace mecom2
} // namespace menps

