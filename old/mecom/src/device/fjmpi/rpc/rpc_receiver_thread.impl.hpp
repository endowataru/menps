
#pragma once

#include "common/rpc/rpc_invoker.impl.hpp"
#include "rpc_notifier.impl.hpp"
#include "rpc_sender.hpp"
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/ult.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_receiver_thread
{
public:
    void initialize(rpc_connection_pool& pool, rpc_notifier& notifier, rpc_invoker& invoker)
    {
        pool_ = &pool;
        notifier_ = &notifier;
        invoker_ = &invoker;
        
        th_ = mefdn::thread(
            mefdn::bind1st_of_1(
                MEFDN_MAKE_INLINED_FUNCTION(&rpc_receiver_thread::start)
            ,   mefdn::wrap_reference(*this)
            )
        );
    }
    
    void finalize()
    {
        th_.join();
    }
    
private:
    static void start(rpc_receiver_thread& self)
    {
        mefdn::unique_lock<mefdn::mutex> lc(self.notifier_->get_mutex());
        
        process_id_t sender_proc;
        int local_nic;
        
        MEFDN_LOG_DEBUG("msg:Waiting for RPC request...");
        
        while (self.notifier_->wait_for_request(lc, &sender_proc, &local_nic))
        {
            ticket_t receiver_count;
            
            MEFDN_LOG_DEBUG(
                "msg:Started processing RPC."
                "sender_proc:{}\tlocal_nic:{}"
            ,   sender_proc
            ,   local_nic
            );
            
            // Detect the buffer index.
            const message_buffer& request_buf = self.pool_->start_receiving(sender_proc, local_nic, &receiver_count);
            
            lc.unlock();
            
            self.process(sender_proc, request_buf);
            
            self.pool_->finish_receiving(sender_proc, local_nic, receiver_count);
            
            MEFDN_LOG_DEBUG(
                "msg:Finished processing RPC. Wait for RPC request again..."
                "sender_proc:{}\tlocal_nic:{}"
            ,   sender_proc
            ,   local_nic
            );
            
            lc.lock();
        }
    }
    
    void process(
        const process_id_t      sender_proc
    ,   const message_buffer&   buf
    ) {
        if (buf.is_reply)
        {
            std::memcpy(buf.return_ptr, buf.data, buf.data_size);
            
            // Execute the callback.
            buf.on_complete();
        }
        else {
            rpc_message_data data;
            
            const index_t reply_size
                = invoker_->call(
                    sender_proc
                ,   buf.handler_id
                ,   buf.data
                ,   buf.data_size
                ,   data
                ,   MECOM_RPC_MAX_DATA_SIZE
                );
            
            while (!rpc::untyped::try_send_reply(sender_proc, buf, data, reply_size))
            {
                mefdn::ult::this_thread::yield();
            }
        }
    }
    
    mefdn::thread          th_;
    rpc_connection_pool*    pool_;
    rpc_notifier*           notifier_;
    rpc_invoker*            invoker_;
};

} // unnamed namespace

} // namespace rpc
} // namespace fjmpi
} // namespace mecom
} // namespace menps

