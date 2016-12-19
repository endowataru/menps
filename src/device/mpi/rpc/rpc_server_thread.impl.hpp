
#pragma once

#include "rpc.hpp"
#include "common/rpc/rpc_invoker.impl.hpp"
#include <mgbase/mutex.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/functional/bind.hpp>
#include <mgbase/functional/reference_wrapper.hpp>
#include <mgbase/functional/functional_constant.hpp>

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_server_thread
{
public:
    void initialize(mpi_interface& mi, rpc_invoker& invoker, const MPI_Comm comm, const int tag)
    {
        mi_ = &mi;
        
        invoker_ = &invoker;
        
        comm_ = comm;
        tag_ = tag;
        
        finished_ = false;
        
        th_ = mgbase::thread(
            mgbase::bind1st_of_1(
                MGBASE_MAKE_INLINED_FUNCTION(&rpc_server_thread::start)
            ,   mgbase::wrap_reference(*this)
            )
        );
    }
    
    void finalize()
    {
        {
            mgbase::unique_lock<mgbase::mutex> lc(mtx_);
            finished_ = true;
            cv_.notify_one();
        }
        
        th_.join();
    }

private:
    static void start(rpc_server_thread& self)
    {
        while (!self.finished_)
        {
            int client_rank;
            message_buffer request_buf;
            
            if (self.recv_request(&client_rank, &request_buf))
            {
                mgbase::uint8_t reply_data[MGCOM_RPC_MAX_DATA_SIZE];
                self.call(client_rank, request_buf, reply_data);
                
                self.send_reply(client_rank, request_buf.reply_tag, reply_data, request_buf.reply_size);
            }
        }
    }
    
    struct call_notify
    {
        rpc_server_thread*  self;
        
        void operator() () const {
            self->notify();
        }
    };
    
    bool recv_request(
        int* const              client_rank
    ,   message_buffer* const   request_buf
    ) {
        mgbase::unique_lock<mgbase::mutex> lc(this->mtx_);
        this->ready_ = false;
        
        MPI_Status status;
        
        mi_->irecv({
            request_buf
        ,   sizeof(message_buffer)
        ,   MPI_ANY_SOURCE
        ,   this->get_tag()
        ,   comm_
        ,   &status
        ,   call_notify{ this }
        });
        
        while (!this->ready_)
        {
            if (this->finished_)
                return false;
            
            this->cv_.wait(lc);
        }
        
        if (this->finished_)
            return false;
        
        *client_rank = status.MPI_SOURCE;
        
        MGBASE_LOG_DEBUG(
            "msg:Received RPC request.\t"
            "client_rank:{}"
        ,   *client_rank
        );
        
        return true;
    }
    
    void call(
        const int               client_rank
    ,   const message_buffer&   request_buf
    ,   void* const             reply_data
    ) {
        const index_t reply_size
            = invoker_->call(
                static_cast<process_id_t>(client_rank)
            ,   request_buf.id
            ,   request_buf.data
            ,   request_buf.size
            ,   reply_data
            ,   MGCOM_RPC_MAX_DATA_SIZE
            );
        
        MGBASE_ASSERT(request_buf.reply_size >= static_cast<int>(reply_size));
    }
    
    void send_reply(const int client_rank, const int reply_tag, const void* const reply_data, const int reply_size)
    {
        mgbase::ult::sync_flag flag;
        
        mi_->isend({
            reply_data
        ,   reply_size
        ,   client_rank
        ,   reply_tag
        ,   comm_
        ,   mgbase::make_callback_notify(&flag)
        });
        
        flag.wait();
    }
    
    void notify() {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        ready_ = true;
        cv_.notify_one();
    }
    
    int get_tag() const MGBASE_NOEXCEPT {
        return tag_;
    }
    
    mpi_interface*              mi_;
    rpc_invoker*                invoker_;
    
    mgbase::thread              th_;
    bool                        finished_;
    
    mgbase::mutex               mtx_;
    mgbase::condition_variable  cv_;
    bool                        ready_;
    
    MPI_Comm                    comm_;
    int                         tag_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mpi
} // namespace mgcom

