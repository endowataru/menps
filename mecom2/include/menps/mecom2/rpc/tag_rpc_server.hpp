
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class tag_rpc_server
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    using process_id_type = typename P::process_id_type;
    
    using server_request_message_type = typename P::template server_request_message<void>;
    
public:
    MEFDN_NODISCARD
    bool try_progress()
    {
        auto& self = this->derived();
        auto& rqst_comm = self.get_request_comm();
        
        server_request_message_type rqst_msg;
        process_id_type src_proc;
        
        // Receive a request.
        if (
            ! rqst_comm.try_recv_request(&rqst_msg, &src_proc)
            // MPI_Iprobe -> MPI_Irecv
        ) {
            return false;
        }
        
        const auto header = rqst_msg.header();
        const auto handler_id = header->handler_id;
        
        auto& inv = self.get_invoker();
        
        auto rply_msg =
            inv.call(self.get_server(), handler_id, src_proc, mefdn::move(rqst_msg));
        
        auto& rply_comm = self.get_reply_comm();
        const auto rply_tag = header->reply_tag;
        
        // Send a reply.
        rply_comm.send_reply(mefdn::move(rply_msg), src_proc, rply_tag);
        
        return true;
    }
};

} // namespace mecom2
} // namespace menps

