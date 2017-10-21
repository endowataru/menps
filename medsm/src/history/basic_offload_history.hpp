
#pragma once

#include "medsm_common.hpp"
#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/memory/shared_ptr.hpp>
#include <menps/mefdn/mutex.hpp>
#include <deque>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_offload_history
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::abs_block_id_type  abs_block_id_type;
    typedef typename Policy::callback_type      callback_type;
    
    typedef typename Policy::spinlock           spinlock_type;
    
    typedef typename Policy::mutex              mutex_type;
    typedef typename Policy::condition_variable cv_type;
    typedef typename Policy::unique_mutex_lock  unique_lock_type;
    
    typedef typename Policy::thread             thread_type;
    
public:
    basic_offload_history()
        : finished_{false}
        , cur_epoch_(mefdn::make_shared<epoch_info>())
    {
    }
    
protected:
    void start()
    {
        this->th_ = thread_type(starter{*this});
    }
    
    void stop()
    {
        this->finished_.store(true);
        
        {
            mefdn::lock_guard<mutex_type> lk(this->mtx_);
            this->cv_.notify_one();
        }
               
        this->th_.join();
    }
    
public:
    void add(const abs_block_id_type ablk_id)
    {
        {
            mefdn::lock_guard<spinlock_type> lk(this->cur_lock_);
            this->cur_epoch_->ids.push_back(ablk_id);
        }
    }
    
    void commit(callback_type cb)
    {
        auto epoch = mefdn::make_shared<epoch_info>();
        
        {
            mefdn::lock_guard<spinlock_type> lk(this->cur_lock_);
            mefdn::swap(epoch, this->cur_epoch_);
        }
        
        epoch->cb = mefdn::move(cb);
        
        {
            mefdn::lock_guard<mutex_type> lk(this->mtx_);
            this->eps_.push_back(mefdn::move(epoch));
            this->cv_.notify_one();
        }
    }
    
private:
    struct starter
    {
       basic_offload_history& self;
       
       void operator() () {
            self.loop();
       }
    };
    
    void loop()
    {
        auto& self = this->derived();
        
        unique_lock_type lk(this->mtx_);
        
        while (true) {
            if (eps_.empty()) {
                if (this->finished_.load(mefdn::memory_order_relaxed)) {
                    return;
                }
                else {
                    this->cv_.wait(lk);
                }
            }
            else {
                auto ep = mefdn::move(this->eps_.front());
                this->eps_.pop_front();
                
                lk.unlock();
                
                MEFDN_RANGE_BASED_FOR(const auto& id, ep->ids)
                {
                    self.downgrade(id);
                }
                
                ep->cb();
                
                ep.reset();
                
                lk.lock();
            }
        }
    }
    
    struct epoch_info {
        std::vector<abs_block_id_type>  ids;
        callback_type                   cb;
    };
    
    typedef mefdn::shared_ptr<epoch_info>  epoch_ptr_type;
    
    mefdn::atomic<bool> finished_;
    
    mefdn::spinlock cur_lock_;
    epoch_ptr_type cur_epoch_; // guarded by cur_lock_
    
    std::deque<epoch_ptr_type> eps_; // guarded by mtx_
    
    mutex_type  mtx_;
    cv_type     cv_;
    
    thread_type th_;
};

} // namespace medsm
} // namespace menps

