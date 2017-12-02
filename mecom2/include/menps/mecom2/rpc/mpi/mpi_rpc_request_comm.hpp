
#pragma once

#include "mpi.hpp"
#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class mpi_rpc_request_comm
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    using process_id_type = typename P::process_id_type;
    
    using server_request_message_type = typename P::template server_request_message<void>;
    using client_request_message_type = typename P::template client_request_message<void>;
    
public:
    void send_request(client_request_message_type msg, const process_id_type proc)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto tag = self.get_request_tag();
        const auto comm = self.get_request_comm();
        
        mi.send({ msg.header(), msg.total_size_in_bytes(),
            proc, tag, comm });
    }
    
    MEFDN_NODISCARD
    bool try_recv_request(
        server_request_message_type* const  out_msg
    ,   process_id_type* const              out_proc
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto tag = self.get_request_tag();
        const auto comm = self.get_request_comm();
        
        MPI_Status status{};
        if (! mi.iprobe({ MPI_ANY_SOURCE, tag, comm, &status })) {
            return false;
        }
        
        const auto src_rank = status.MPI_SOURCE;
        const auto count = mi.get_count(status);
        
        const auto data_size =
            count - server_request_message_type::header_size();
        
        auto msg = self.allocate_server_request(data_size);
        
        mi.recv({ msg.header(), count,
            src_rank, tag, comm, &status });
        
        *out_msg = mefdn::move(msg);
        *out_proc = src_rank;
        
        return true;
    }
};

} // namespace mecom2
} // namespace menps

