
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class tag_rpc_client
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    using process_id_type = typename P::process_id_type;
    using handler_id_type = typename P::handler_id_type;
    
    using client_request_message_type = typename P::template client_request_message<void>;
    using client_reply_message_type = typename P::template client_reply_message<void>;
    
public:
    struct call_params {
        process_id_type                 server_proc;
        handler_id_type                 handler_id;
        client_request_message_type&&   rqst_msg;
    };
    
    // Note: Added for GCC 4.8
    client_reply_message_type untyped_call(
        process_id_type                 server_proc
    ,   handler_id_type                 handler_id
    ,   client_request_message_type&&   rqst_msg
    ) {
        return this->untyped_call(call_params{
            server_proc
        ,   handler_id
        ,   mefdn::move(rqst_msg)
        });
    }
    
    client_reply_message_type untyped_call(const call_params& p)
    {
        auto& self = this->derived();
        
        auto& rqst_comm = self.get_request_comm();
        
        #if 0
        if (p.server_proc == rqst_comm.current_process_id()) {
            auto& inv = self.get_invoker();
            
            auto ret =
                inv.call(self.get_server(), p.handler_id, p.server_proc, mefdn::move(p.rqst_msg));
            
            return ret;
        }
        #endif
        
        auto rply_tag = self.allocate_tag();
        
        {
            const auto header = p.rqst_msg.header();
            header->handler_id = p.handler_id;
            header->reply_tag = rply_tag;
            
            // Send a request.
            rqst_comm.send_request(mefdn::move(p.rqst_msg), p.server_proc);
        }
        
        client_reply_message_type rply_msg;
        
        {
            auto& rply_comm = self.get_reply_comm();
            
            // Receive a reply.
            rply_msg = rply_comm.recv_reply(p.server_proc, rply_tag);
        }
        
        self.deallocate_tag(mefdn::move(rply_tag));
        
        return rply_msg;
    }
};

} // namespace mecom2
} // namespace menps

