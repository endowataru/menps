
#pragma once

#include "rpc.hpp"
#include "common/rpc/rpc_invoker.impl.hpp"
#include <mgcom/ult.hpp>

namespace mgcom {
namespace mpi {

class rpc_server_thread
{
    typedef ult::mutex      mutex_type;
    
public:
    struct conf
    {
        mpi_interface*              mi;
        rpc::rpc_invoker*           invoker;
        
        MPI_Comm                    comm;
        int                         tag;
    };
    
    explicit rpc_server_thread(const conf& c)
        : conf_(c)
        , finished_{false}
        , mtx_{}
        , cv_{}
        , ready_(false)
        , th_{ loop{*this} }
    {}
    
    ~rpc_server_thread()
    {
        this->finished_.store(true);
        
        {
            mgbase::unique_lock<mutex_type> lk(this->mtx_);
            cv_.notify_one();
        }
        
        th_.join();
    }
    
private:
    struct loop
    {
        rpc_server_thread&  self;
        
        void operator() () const
        {
            self.do_loop();
        }
    };
    
    void do_loop()
    {
        while (!is_finished())
        {
            rpc::message_buffer buf;
            
            const auto ret = this->recv_request(&buf);
            
            if (ret.valid)
            {
                mgbase::uint8_t reply_data[MGCOM_RPC_MAX_DATA_SIZE];
                
                this->call(ret.client_rank, buf, reply_data);
                
                this->send_reply(ret.client_rank, buf.reply_tag, reply_data, buf.reply_size);
            }
        }
    }
    
    struct call_notify
    {
        rpc_server_thread&  self;
        
        void operator() () const
        {
            self.notify();
        }
    };
    
    void notify()
    {
        // Callback function for the completion of MPI_Irecv().
        
        ult::unique_lock<mutex_type> lc(this->mtx_);
        
        // The buffer is now ready.
        this->ready_ = true;
        
        // Notify the thread to proceed.
        this->cv_.notify_one();
    }
    
    struct recv_result
    {
        bool valid;
        int  client_rank;
    };
    
    recv_result recv_request(rpc::message_buffer* const buf)
    {
        ult::unique_lock<ult::mutex> lc(this->mtx_);
        
        this->ready_ = false;
        
        MPI_Status status;
        
        this->conf_.mi->irecv({
            buf
        ,   sizeof(rpc::message_buffer)
        ,   MPI_ANY_SOURCE
        ,   this->conf_.tag
        ,   this->conf_.comm
        ,   &status
        ,   call_notify{ *this }
        });
        
        while (!this->ready_)
        {
            if (is_finished()) {
                return { false, MPI_PROC_NULL };
            }
            
            this->cv_.wait(lc);
        }
        
        if (is_finished()) {
            return { false, MPI_PROC_NULL };
        }
        
        const int client_rank = status.MPI_SOURCE;
        
        MGBASE_LOG_DEBUG(
            "msg:Received RPC request.\t"
            "client_rank:{}"
        ,   client_rank
        );
        
        return { true, client_rank };
    }
    
    void call(
        const int                   client_rank
    ,   const rpc::message_buffer&  buf
    ,   void* const                 reply_data
    ) {
        const auto reply_size
            = this->conf_.invoker->call(
                static_cast<process_id_t>(client_rank)
            ,   buf.id
            ,   buf.data
            ,   buf.size
            ,   reply_data
            ,   MGCOM_RPC_MAX_DATA_SIZE
            );
        
        MGBASE_ASSERT(buf.reply_size >= static_cast<int>(reply_size));
    }
    
    void send_reply(
        const int           client_rank
    ,   const int           reply_tag
    ,   const void* const   reply_data
    ,   const int           reply_size
    )
    {
        ult::sync_flag flag;
        
        this->conf_.mi->isend({
            reply_data
        ,   reply_size
        ,   client_rank
        ,   reply_tag
        ,   this->conf_.comm
        ,   mgbase::make_callback_notify(&flag)
        });
        
        flag.wait();
    }
    
    bool is_finished() const MGBASE_NOEXCEPT
    {
        return this->finished_.load(mgbase::memory_order_relaxed);
    }
    
    const conf conf_;
    
    mgbase::atomic<bool>    finished_;
    
    ult::mutex              mtx_;
    ult::condition_variable cv_;
    bool                    ready_;
    
    ult::thread             th_;
};

} // namespace mpi
} // namespace mgcom

