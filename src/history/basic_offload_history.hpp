
#pragma once

#include "mgdsm_common.hpp"
#include <mgbase/crtp_base.hpp>
#include <mgbase/shared_ptr.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <deque>

namespace mgdsm {

template <typename Policy>
class basic_offload_history
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
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
        , cur_epoch_(mgbase::make_shared<epoch_info>())
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
            mgbase::lock_guard<mutex_type> lk(this->mtx_);
            this->cv_.notify_one();
        }
               
        this->th_.join();
    }
    
public:
    void add(const abs_block_id_type ablk_id)
    {
        {
            mgbase::lock_guard<spinlock_type> lk(this->cur_lock_);
            this->cur_epoch_->ids.push_back(ablk_id);
        }
    }
    
    void commit(callback_type cb)
    {
        auto epoch = mgbase::make_shared<epoch_info>();
        
        {
            mgbase::lock_guard<spinlock_type> lk(this->cur_lock_);
            mgbase::swap(epoch, this->cur_epoch_);
        }
        
        epoch->cb = mgbase::move(cb);
        
        {
            mgbase::lock_guard<mutex_type> lk(this->mtx_);
            this->eps_.push_back(mgbase::move(epoch));
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
                if (this->finished_.load(mgbase::memory_order_relaxed)) {
                    return;
                }
                else {
                    this->cv_.wait(lk);
                }
            }
            else {
                auto ep = mgbase::move(this->eps_.front());
                this->eps_.pop_front();
                
                lk.unlock();
                
                MGBASE_RANGE_BASED_FOR(const auto& id, ep->ids)
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
    
    typedef mgbase::shared_ptr<epoch_info>  epoch_ptr_type;
    
    mgbase::atomic<bool> finished_;
    
    mgbase::spinlock cur_lock_;
    epoch_ptr_type cur_epoch_; // guarded by cur_lock_
    
    std::deque<epoch_ptr_type> eps_; // guarded by mtx_
    
    mutex_type  mtx_;
    cv_type     cv_;
    
    thread_type th_;
};

} // namespace mgdsm

