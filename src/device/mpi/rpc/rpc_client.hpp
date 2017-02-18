
#pragma once

#include "rpc_base.hpp"
#include <mgcom/ult.hpp>
#include <mgbase/container/circular_buffer.hpp>

namespace mgcom {
namespace mpi {

class rpc_client
    : public virtual rpc_base
{
    typedef ult::mutex          mutex_type;
    typedef rpc_message_buffer  buffer_type;
    
    static const mgbase::size_t num_tags = 1000;
    
    typedef rpc::server_request_message<void>   server_request_message_type;
    typedef rpc::server_reply_message<void>     server_reply_message_type;
    typedef rpc::client_request_message<void>   request_message_type;
    typedef rpc::client_reply_message<void>     reply_message_type;
    
public:
    rpc_client(mpi_interface& mi, endpoint& ep)
        : rpc_base(mi)
        , ep_(ep)
        , tags_(mgbase::make_unique<tag_info []>(num_tags))
    {
        for (mgbase::size_t i = 0; i < num_tags; ++i) {
            free_ids_.push_back(i);
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<rpc::client_reply_message<void>>
    async_call(const rpc::untyped::async_call_params& params) MGBASE_OVERRIDE
    {
        MGBASE_ASSERT(valid_process_id(ep_, params.server_proc));
        MGBASE_ASSERT(params.handler_id < MGCOM_RPC_MAX_NUM_HANDLERS);
        MGBASE_ASSERT(params.rqst_msg.size_in_bytes() <= MGCOM_RPC_MAX_DATA_SIZE);
        //MGBASE_ASSERT(params.arg_ptr != MGBASE_NULLPTR);
        //MGBASE_ASSERT(params.return_size == 0 || params.return_ptr != MGBASE_NULLPTR);
        
        // Avoid loopback and deadlocking.
        if (params.server_proc == mgcom::current_process_id())
        {
            auto ret =
                this->get_invoker().call(
                    params.handler_id
                ,   params.server_proc
                ,   server_request_message_type::convert_from(
                        mgbase::move(params.rqst_msg)
                    )
                );
            
            return ult::make_async_ready(
                reply_message_type::convert_from(
                    mgbase::move(ret.rply_msg)
                )
            );
        }
        
        const auto id = this->allocate_id();
        
        // Set the callback.
        this->tags_[id].on_complete = params.on_complete;
        
        const auto send_tag = this->get_send_tag(params.handler_id);
        const auto recv_tag = this->get_recv_tag(id);
        
        const auto comm = this->get_comm();
        
        auto& mi = this->get_mpi_interface();
        
        {
            auto rply_msg =
                reply_message_type::convert_from(
                    rpc::detail::allocate_message(16 /*TODO*/, MGCOM_RPC_MAX_DATA_SIZE)
                );
            
            mi.recv_async(
                rply_msg.get()                              // buf
            ,   static_cast<int>(rply_msg.size_in_bytes())  // size_in_bytes
            ,   static_cast<int>(params.server_proc)        // src_proc
            ,   recv_tag                                    // tag
            ,   comm                                        // comm
            ,   MPI_STATUS_IGNORE                           // status_result
            ,   recv_reply{ *this, id }                     // on_complete
            );
            
            this->tags_[id].rply_msg = mgbase::move(rply_msg);
        }
        
        {
            const auto msg_buf =
                mgbase::make_unique<buffer_type>();
            
            const auto ptr = params.rqst_msg.get();
            const auto size = params.rqst_msg.size_in_bytes();
            
            msg_buf->id      = params.handler_id;
            msg_buf->size    = size;
            
            msg_buf->reply_size  = MGCOM_RPC_MAX_DATA_SIZE;
            msg_buf->reply_tag   = recv_tag;
            
            std::memcpy(msg_buf->data, ptr, size);
            
            //mpi::irsend( // TODO: Introduce buffer management
            mi.send(
                msg_buf.get()                               // buf
            ,   static_cast<int>(sizeof(buffer_type))       // size_in_bytes
            ,   static_cast<int>(params.server_proc)        // dest_proc
            ,   send_tag                                    // tag
            ,   comm                                        // comm
            );
            
            // TODO: Change send to send_async.
            //       The buffer must be preserved until "send" is being processed.
            
            // msg_buf is deallocated here.
        }
        
        return ult::make_async_deferred<rpc::client_reply_message<void>>();
    }
    
private:
    mgbase::size_t allocate_id()
    {
        while (true)
        {
            {
                ult::lock_guard<mutex_type> lk(this->mtx_);
                
                if (! this->free_ids_.empty())
                {
                    // Dequeue from the free list.
                    const auto id = this->free_ids_.front();
                    this->free_ids_.pop_front();
                    
                    return id;
                }
            }
            
            ult::yield();
        }
    }
    
    void return_id(const mgbase::size_t id)
    {
        ult::lock_guard<mutex_type> lk(this->mtx_);
        
        MGBASE_ASSERT(!this->free_ids_.full());
        
        this->free_ids_.push_back(id);
    }
    
    struct recv_reply
    {
        rpc_client&     self;
        mgbase::size_t  id;
        
        void operator() () const
        {
            self.return_id(id);
            
            self.tags_[id].on_complete(
                mgbase::move(self.tags_[id].rply_msg)
            );
        }
    };
    
    struct tag_info
    {
        rpc::untyped::call_callback_t   on_complete;
        reply_message_type              rply_msg;
    };
    
    endpoint&       ep_;
    
    mutex_type mtx_;
    mgbase::static_circular_buffer<mgbase::size_t, num_tags> free_ids_;
    mgbase::unique_ptr<tag_info []> tags_;
};

} // namespace mpi
} // namespace mgcom

