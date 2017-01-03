
#pragma once

#include "rpc.hpp"
#include "common/rpc/rpc_invoker.impl.hpp"
#include <mgcom/ult.hpp>

namespace mgcom {
namespace mpi {

class rpc_server_thread
{
    typedef ult::mutex                          mutex_type;
    typedef rpc_message_buffer                  buffer_type;
    typedef mgbase::unique_ptr<buffer_type>     buffer_ptr_type;
    
public:
    struct config
    {
        mpi_interface*      mi;
        rpc::rpc_invoker*   invoker;
        
        MPI_Comm            comm;
        int                 tag;
    };
    
    explicit rpc_server_thread(const config& conf)
        : conf_(conf)
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
            auto buf_ptr = mgbase::make_unique<buffer_type>();
            
            const auto ret = this->recv_request(*buf_ptr);
            
            if (ret.valid)
            {
                ult::thread th(
                    call_functor{ *this, ret.cli_rank, mgbase::move(buf_ptr) }
                );
                
                th.detach();
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
        int  cli_rank;
    };
    
    recv_result recv_request(buffer_type& buf)
    {
        ult::unique_lock<ult::mutex> lc(this->mtx_);
        
        this->ready_ = false;
        
        MPI_Status status;
        
        this->conf_.mi->recv_async({
            {
                &buf
            ,   sizeof(buffer_type)
            ,   MPI_ANY_SOURCE
            ,   this->conf_.tag
            ,   this->conf_.comm
            ,   &status
            }
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
        
        const int cli_rank = status.MPI_SOURCE;
        
        MGBASE_LOG_DEBUG(
            "msg:Received RPC request.\t"
            "cli_rank:{}"
        ,   cli_rank
        );
        
        return { true, cli_rank };
    }
    
    struct call_functor
    {
        rpc_server_thread&  self;
        int                 cli_rank;
        buffer_ptr_type     buf_ptr;
        
        void operator() ()
        {
            auto& buf = *buf_ptr;
            
            mgbase::uint8_t reply_data[MGCOM_RPC_MAX_DATA_SIZE];
            
            self.call(cli_rank, buf, reply_data);
            
            self.send_reply(cli_rank, buf.reply_tag, reply_data, buf.reply_size);
        }
    };
    
    void call(
        const int           cli_rank
    ,   const buffer_type&  buf
    ,   void* const         reply_data
    ) {
        const auto reply_size
            = this->conf_.invoker->call(
                static_cast<process_id_t>(cli_rank)
            ,   buf.id
            ,   buf.data
            ,   buf.size
            ,   reply_data
            ,   MGCOM_RPC_MAX_DATA_SIZE
            );
        
        MGBASE_ASSERT(buf.reply_size >= static_cast<int>(reply_size));
    }
    
    void send_reply(
        const int           cli_rank
    ,   const int           reply_tag
    ,   const void* const   reply_data
    ,   const int           reply_size
    )
    {
        ult::sync_flag flag;
        
        this->conf_.mi->send_async({
            {
                reply_data
            ,   reply_size
            ,   cli_rank
            ,   reply_tag
            ,   this->conf_.comm
            }
        ,   mgbase::make_callback_notify(&flag)
        });
        
        flag.wait();
    }
    
    bool is_finished() const MGBASE_NOEXCEPT
    {
        return this->finished_.load(mgbase::memory_order_relaxed);
    }
    
    const config conf_;
    
    mgbase::atomic<bool>    finished_;
    
    ult::mutex              mtx_;
    ult::condition_variable cv_;
    bool                    ready_;
    
    ult::thread             th_;
};

} // namespace mpi
} // namespace mgcom

