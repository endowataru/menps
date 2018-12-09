
#pragma once

#include "rpc_connection_pool.hpp"
#include "device/fjmpi/rma/rma.hpp"
#include <menps/mecom/rma.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>
#include <menps/mefdn/container/circular_buffer.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_notifier
{
    static const index_t max_num_connections = 16;
    static const index_t max_num_notifications = 10240;
    
    struct notification
    {
        process_id_t    sender_proc;
        int             local_nic;
    };
    
public:
    void initialize()
    {
        finished_ = false;
        
        queue_.set_capacity(max_num_notifications);
    }
    
    void set_finished()
    {
        mefdn::unique_lock<mefdn::mutex> lc(mtx_);
        finished_ = true;
        cv_.notify_all();
    }
    
    void finalize()
    {
        // do nothing
    }
    
    void notify(
        const process_id_t  sender_proc
    ,   const int           local_nic
    ) noexcept
    {
        const notification notif = { sender_proc, local_nic };
        
        mefdn::unique_lock<mefdn::mutex> lc(mtx_);
        
        while (queue_.full())
        {
            lc.unlock();
            
            // TODO : insert wait
            
            lc.lock();
        }
        
        queue_.push_back(notif);
        
        cv_.notify_one();
    }
    
    mefdn::mutex& get_mutex() noexcept {
        return mtx_;
    }
    
    bool wait_for_request(
        mefdn::unique_lock<mefdn::mutex>& lc
    ,   process_id_t* const                 sender_proc_result
    ,   int* const                          local_nic_result
    ) noexcept
    {
        MEFDN_ASSERT(lc.owns_lock());
        
        while (queue_.empty()) {
            if (finished_)
                return false;
            
            cv_.wait(lc);
        }
        
        const notification& notif = queue_.front();
        
        *sender_proc_result = notif.sender_proc;
        *local_nic_result   = notif.local_nic;
        
        queue_.pop_front();
        
        return true;
    }
    
private:
    mefdn::mutex                           mtx_;
    mefdn::condition_variable              cv_;
    mefdn::circular_buffer<notification>   queue_;
    bool                                    finished_;
};

} // unnamed namespace

} // namespace rpc
} // namespace fjmpi
} // namespace mecom
} // namespace menps

