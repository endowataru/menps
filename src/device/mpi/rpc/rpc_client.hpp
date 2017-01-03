
#pragma once

#include "rpc_base.hpp"

namespace mgcom {
namespace mpi {

class rpc_client
    : public virtual rpc_base
{
    typedef rpc_message_buffer  buffer_type;
    
public:
    rpc_client(mpi_interface& mi, endpoint& ep)
        : mi_(mi)
        , ep_(ep)
    { }
    
    virtual bool try_call_async(const rpc::untyped::call_params& params) MGBASE_OVERRIDE
    {
        MGBASE_ASSERT(valid_process_id(ep_, params.proc));
        MGBASE_ASSERT(params.handler_id < MGCOM_RPC_MAX_NUM_HANDLERS);
        MGBASE_ASSERT(params.arg_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(params.return_size == 0 || params.return_ptr != MGBASE_NULLPTR);
        
        const auto send_tag = get_send_tag();
        const auto recv_tag = get_recv_tag();
        
        const auto comm = this->get_comm();
        
        buffer_type msg_buf;
        
        msg_buf.id      = params.handler_id;
        msg_buf.size    = params.arg_size;
        
        msg_buf.reply_size  = static_cast<int>(params.return_size);
        msg_buf.reply_tag   = recv_tag;
        
        std::memcpy(msg_buf.data, params.arg_ptr, params.arg_size);
        
        ult::sync_flag send_finished;
        
        //mpi::irsend( // TODO: Introduce buffer management
        mi_.send_async({
            {
                &msg_buf                                    // buf
            ,   static_cast<int>(sizeof(buffer_type))       // size_in_bytes
            ,   static_cast<int>(params.proc)               // dest_proc
            ,   send_tag                                    // tag
            ,   comm                                        // comm
            }
        ,   mgbase::make_callback_notify(&send_finished)    // on_complete
        });
        
        mi_.recv_async({
            {
                params.return_ptr                       // buf
            ,   static_cast<int>(params.return_size)    // size_in_bytes
            ,   static_cast<int>(params.proc)           // dest_proc
            ,   recv_tag                                // tag
            ,   comm                                    // comm
            ,   MPI_STATUS_IGNORE                       // status_result
            }
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
};

} // namespace mpi
} // namespace mgcom

