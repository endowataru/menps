
#pragma once

#include "rpc.hpp"
#include <mgbase/thread.hpp>
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_client
{
public:
    rpc_client(mpi_interface& mi, endpoint& ep, const MPI_Comm comm)
        : mi_(mi)
        , ep_(ep)
        , comm_(comm) { }
    
    bool try_call_async(const untyped::call_params& params)
    {
        MGBASE_ASSERT(valid_process_id(ep_, params.proc));
        MGBASE_ASSERT(params.handler_id < MGCOM_RPC_MAX_NUM_HANDLERS);
        MGBASE_ASSERT(params.arg_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(params.return_size == 0 || params.return_ptr != MGBASE_NULLPTR);
        
        const int send_tag = get_send_tag();
        const int recv_tag = get_recv_tag();
        
        message_buffer msg_buf;
        
        msg_buf.id      = params.handler_id;
        msg_buf.size    = params.arg_size;
        
        msg_buf.reply_size  = static_cast<int>(params.return_size);
        msg_buf.reply_tag   = recv_tag;
        
        std::memcpy(msg_buf.data, params.arg_ptr, params.arg_size);
        
        ult::sync_flag send_finished;
        
        //mpi::irsend( // TODO: Introduce buffer management
        mi_.isend({
            &msg_buf                                        // buf
        ,   static_cast<int>(sizeof(message_buffer))        // size_in_bytes
        ,   static_cast<int>(params.proc)                   // dest_proc
        ,   send_tag                                        // tag
        ,   comm_                                           // comm
        ,   mgbase::make_callback_notify(&send_finished)    // on_complete
        });
        
        mi_.irecv({
            params.return_ptr                       // buf
        ,   static_cast<int>(params.return_size)    // size_in_bytes
        ,   static_cast<int>(params.proc)           // dest_proc
        ,   recv_tag                                // tag
        ,   comm_                                   // comm
        ,   MPI_STATUS_IGNORE                       // status_result
        ,   params.on_complete                      // on_complete
        });
        
        send_finished.wait();
        
        return true;
    }
    
private:
    int get_send_tag() {
        return 100; // FIXME
    }
    
    int get_recv_tag() {
        return 200; // FIXME
    }
    
    mpi_interface& mi_;
    endpoint& ep_;
    MPI_Comm comm_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mpi
} // namespace mgcom

