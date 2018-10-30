
#pragma once

#include "mpi.hpp"
#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class mpi_rpc_reply_comm
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    using process_id_type = typename P::process_id_type;
    using tag_type = typename P::tag_type;
    
    using server_reply_message_type = typename P::template server_reply_message<void>;
    using client_reply_message_type = typename P::template client_reply_message<void>;
    
public:
    void send_reply(server_reply_message_type msg, const process_id_type proc, const tag_type tag)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto comm = self.get_reply_comm();
        
        mi.send({
            msg.header(),
            static_cast<int>(msg.total_size_in_bytes()),
            MPI_BYTE,
            proc, tag, comm });
    }
    
    client_reply_message_type recv_reply(const process_id_type proc, const tag_type tag)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto comm = self.get_reply_comm();
        
        MPI_Status status = MPI_Status();
        mi.probe({ proc, tag, comm, &status });
        
        int count = 0;
        mi.get_count({ &status, MPI_BYTE, &count });
        
        auto msg = self.allocate_client_reply(count);
        
        mi.recv({ msg.header(), count, MPI_BYTE,
            proc, tag, comm, &status });
        
        return msg;
    }
};

} // namespace mecom2
} // namespace menps

