
#pragma once

#include "completer.hpp"
#include <unordered_map>
#include <mgcom/ult.hpp>

namespace mgcom {
namespace ibv {

typedef mgbase::uint32_t    qp_num_t;

class completion_selector
{
public:
    completion_selector()
        #ifdef MGCOM_IBV_ENABLE_SLEEP
        : num_outstanding_{0}
        #endif
    { }
    
    completion_selector(const completion_selector&) = delete;
    completion_selector& operator = (const completion_selector&) = delete;
    
    void set(const qp_num_t qp_num, completer& comp)
    {
        MGBASE_ASSERT(m_.find(qp_num) == m_.end());
        
        m_[qp_num] = &comp;
    }
    
    completer& get(const qp_num_t qp_num)
    {
        return *m_[qp_num];
    }
    
    #ifdef MGCOM_IBV_ENABLE_SLEEP
private:
    typedef ult::unique_lock<ult::mutex>    unique_lock_type;
    
public:
    unique_lock_type get_lock() {
        return unique_lock_type(this->mtx_);
    }
    
    bool try_wait(unique_lock_type& lk)
    {
        auto old = num_outstanding_.load(mgbase::memory_order_relaxed);
        
        // If there's no request in QP - CQ
        if (old == 0) {
            // Try to sleep.
            if (num_outstanding_.compare_exchange_weak(old, 1, mgbase::memory_order_relaxed)) {
                cv_.wait(lk);
                
                // Restarted again on notification.
                num_outstanding_.fetch_sub(1, mgbase::memory_order_relaxed);
                return true;
            }
        }
        
        return false;
    }
    
    void remove_outstanding(const mgbase::size_t num_polled, unique_lock_type& lk)
    {
        auto old = num_outstanding_.load(mgbase::memory_order_relaxed);
        
        while (true) {
            // If only there are requests that finished right now
            if (old == (num_polled << 1)) {
                // Try to sleep.
                if (num_outstanding_.compare_exchange_weak(old, 1, mgbase::memory_order_relaxed)) {
                    cv_.wait(lk);
                    
                    // Restarted again on notification.
                    num_outstanding_.fetch_sub(1, mgbase::memory_order_relaxed);
                    break;
                }
                
                // Retry.
                // (old is reloaded by compare_exchange_weak.)
            }
            else {
                // Avoid trying to wait.
                num_outstanding_.fetch_sub(num_polled << 1, mgbase::memory_order_relaxed);
                break;
            }
        }
    }
    
    void notify(const mgbase::size_t num_wrs)
    {
        const auto num_outstanding = num_outstanding_.fetch_add(num_wrs << 1, mgbase::memory_order_acquire);
        
        // Check if LSB is 1 (= sleeping).
        if ((num_outstanding & 1) == 1) {
            force_notify();
        }
    }
    
    void force_notify()
    {
        ult::lock_guard<ult::mutex> lk(this->mtx_);
        // Awake the completer.
        cv_.notify_one();
    }
    #endif
    
private:
    std::unordered_map<qp_num_t, completer*> m_;
    
    #ifdef MGCOM_IBV_ENABLE_SLEEP
    mgbase::atomic<mgbase::size_t>  num_outstanding_;
    
    ult::mutex              mtx_;
    ult::condition_variable cv_;
    #endif
};

} // namespace ibv
} // namespace mgcom

