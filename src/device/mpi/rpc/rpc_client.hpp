
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
    
    static const int tag_start = 1000;
    static const mgbase::size_t num_tags = 1000;
    
public:
    rpc_client(mpi_interface& mi, endpoint& ep)
        : rpc_base(mi)
        , mi_(mi)
        , ep_(ep)
        , tags_(mgbase::make_unique<tag_info []>(num_tags))
    {
        for (mgbase::size_t i = 0; i < num_tags; ++i) {
            free_ids_.push_back(i);
        }
    }
    
    virtual bool try_call_async(const rpc::untyped::call_params& params) MGBASE_OVERRIDE
    {
        MGBASE_ASSERT(valid_process_id(ep_, params.proc));
        MGBASE_ASSERT(params.handler_id < MGCOM_RPC_MAX_NUM_HANDLERS);
        MGBASE_ASSERT(params.arg_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(params.return_size == 0 || params.return_ptr != MGBASE_NULLPTR);
        
        // Avoid loopback and deadlocking.
        if (params.proc == mgcom::current_process_id())
        {
            const auto reply_size = 
                this->get_invoker().call(
                    params.proc
                ,   params.handler_id
                ,   params.arg_ptr
                ,   params.arg_size
                ,   params.return_ptr
                ,   params.return_size
                );
            
            MGBASE_ASSERT(reply_size <= params.return_size);
            
            params.on_complete();
            
            return true;
        }
        
        const auto id = this->allocate_id();
        
        // Set the callback.
        this->tags_[id].on_complete = params.on_complete;
        
        const auto send_tag = this->get_server_tag();
        const auto recv_tag = tag_start + static_cast<int>(id);
        
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
        ,   recv_reply{ *this, id }                     // on_complete
        });
        
        send_finished.wait();
        
        return true;
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
            
            self.tags_[id].on_complete();
        }
    };
    
    int get_send_tag() {    
        return 100; // FIXME
    }
    
    int get_recv_tag() {
        return 200; // FIXME
    }
    
    struct tag_info
    {
        mgbase::callback<void ()> on_complete;
    };
    
    mpi_interface&  mi_;
    endpoint&       ep_;
    
    mutex_type mtx_;
    mgbase::static_circular_buffer<mgbase::size_t, num_tags> free_ids_;
    mgbase::unique_ptr<tag_info []> tags_;
};

} // namespace mpi
} // namespace mgcom

