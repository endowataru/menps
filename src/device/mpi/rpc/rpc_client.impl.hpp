
#pragma once

#include "rpc.hpp"
#include <mgbase/thread.hpp>
#include "device/mpi/mpi_call.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_client
{
public:
    bool try_call(
        const process_id_t          dest_proc
    ,   const handler_id_t          handler_id
    ,   const void* const           arg_ptr
    ,   const index_t               arg_size
    ,   void* const                 return_ptr
    ,   const index_t               return_size
    ,   const mgbase::operation&    on_complete
    ) {
        MGBASE_ASSERT(valid_process_id(dest_proc));
        MGBASE_ASSERT(handler_id < MGCOM_RPC_MAX_NUM_HANDLERS);
        MGBASE_ASSERT(arg_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(return_size == 0 || return_ptr != MGBASE_NULLPTR);
        
        const int send_tag = get_send_tag();
        const int recv_tag = get_recv_tag();
        
        message_buffer msg_buf;
        
        msg_buf.id      = handler_id;
        msg_buf.size    = arg_size;
        
        msg_buf.reply_size  = static_cast<int>(return_size);
        msg_buf.reply_tag   = recv_tag;
        
        std::memcpy(msg_buf.data, arg_ptr, arg_size);
        
        mgbase::atomic<bool> send_finished = MGBASE_ATOMIC_VAR_INIT(false);
        
        //mpi::irsend( // TODO: Introduce buffer management
        mpi::isend(
            &msg_buf                                    // buf
        ,   static_cast<int>(sizeof(message_buffer))    // size_in_bytes
        ,   static_cast<int>(dest_proc)                 // dest_proc
        ,   send_tag                                    // tag
        ,   get_comm()                                  // comm
        ,   mgbase::make_operation_store_release(&send_finished, true)  // on_complete
        );
        
        mpi::irecv(
            return_ptr                      // buf
        ,   static_cast<int>(return_size)   // size_in_bytes
        ,   static_cast<int>(dest_proc)     // dest_proc
        ,   recv_tag                        // tag
        ,   get_comm()                      // comm
        ,   MPI_STATUS_IGNORE               // status_result
        ,   on_complete                     // on_complete
        );
        
        while (!send_finished.load(mgbase::memory_order_acquire)) { }
        
        return true;
    }
    
private:
    MPI_Comm get_comm() {
        return rpc::get_comm();
    }
    
    int get_send_tag() {
        return 100; // FIXME
    }
    
    int get_recv_tag() {
        return 200; // FIXME
    }
};

} // unnamed namespace

} // namespace rpc
} // namespace mpi
} // namespace mgcom

