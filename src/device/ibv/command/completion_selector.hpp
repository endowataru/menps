
#pragma once

#include <mgcom/common.hpp> // be sure to define MGCOM_IBV_ENABLE_SLEEP_CQ
#include "tag_queue.hpp"
#include <unordered_map>
#include <mgcom/ult.hpp>

namespace mgcom {
namespace ibv {

typedef mgbase::uint32_t    qp_num_t;

class completion_selector
{
public:
    completion_selector()
        #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
        : num_outstanding_{0}
        #endif
    { }
    
    completion_selector(const completion_selector&) = delete;
    completion_selector& operator = (const completion_selector&) = delete;
    
    void set(const qp_num_t qp_num, tag_queue& tag_que)
    {
        MGBASE_ASSERT(m_.find(qp_num) == m_.end());
        
        m_[qp_num] = &tag_que;
    }
    
    tag_queue& get(const qp_num_t qp_num)
    {
        return *m_[qp_num];
    }
    
    #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
    
    #ifdef MGCOM_FORK_COMPLETER_THREAD
    
public:
    void set_entrypoint(void* (*func)(void*), void* arg)
    {
        func_ = func;
        arg_ = arg;
    }
    
    void notify(const mgbase::size_t num_wrs)
    {
        const auto num_outstanding = num_outstanding_.fetch_add(num_wrs, mgbase::memory_order_acquire);
        
        if (num_outstanding == 0) {
            MGBASE_LOG_DEBUG(
                "msg:Create completer."
            );
            
            ult::fork_fast(func_, arg_);
        }
    }
    
    void force_notify() {
        // do nothing
    }
    
    bool remove_outstanding_and_try_sleep(const mgbase::size_t num_wrs)
    {
        const auto r = num_outstanding_.fetch_sub(num_wrs, mgbase::memory_order_release);
        MGBASE_ASSERT(r > 0);
        return r == num_wrs;
    }
    
private:
    void* (*func_)(void*);
    void* arg_;
    
    #else
    
private:
    typedef ult::unique_lock<ult::mutex>    unique_lock_type;
    
    unique_lock_type get_lock() {
        return unique_lock_type(this->mtx_);
    }
    
public:
    #if 0
    bool try_wait()
    {
        auto old = num_outstanding_.load(mgbase::memory_order_relaxed);
        
        // If there's no request in QP - CQ
        if (old == 0) {
            auto lk = this->get_lock();
            
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
    
    void remove_outstanding(const mgbase::size_t num_polled)
    {
        auto old = num_outstanding_.load(mgbase::memory_order_relaxed);
        
        while (true) {
            // If only there are requests that finished right now
            if (old == (num_polled << 1)) {
                auto lk = this->get_lock();
                
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
    #endif
    bool try_wait()
    {
        #if 0
        auto old = num_outstanding_.load(mgbase::memory_order_relaxed);
        
        // If there's no request in QP - CQ
        if (old == 0) {
            auto lk = this->get_lock();
            
            // Try to sleep.
            if (num_outstanding_.compare_exchange_weak(old, 1, mgbase::memory_order_relaxed)) {
                cv_.wait(lk);
                
                // Restarted again on notification.
                num_outstanding_.fetch_sub(1, mgbase::memory_order_relaxed);
                return true;
            }
        }
        
        #endif
        return false;
    }
    
    void remove_outstanding(const mgbase::size_t num_wrs)
    {
        auto lk = this->get_lock();
        const auto r = num_outstanding_.fetch_sub(num_wrs, mgbase::memory_order_release);
        MGBASE_ASSERT(r > 0);
        if (r == num_wrs) {
            cv_.wait(lk);
        }
    }
    
    void notify(const mgbase::size_t num_wrs)
    {
        auto lk = this->get_lock();
        const auto num_outstanding = num_outstanding_.fetch_add(num_wrs, mgbase::memory_order_acquire);
        
        if (num_outstanding == 0) {
            MGBASE_LOG_DEBUG(
                "msg:Create completer."
            );
            
            cv_.notify_one();
        }
    }
    
    void force_notify()
    {
        ult::lock_guard<ult::mutex> lk(this->mtx_);
        // Awake the completer.
        cv_.notify_one();
    }
    
private:
    ult::mutex              mtx_;
    ult::condition_variable cv_;
    
    #endif
    
private:
    mgbase::atomic<mgbase::size_t>  num_outstanding_;
    
    #endif
    
private:
    std::unordered_map<qp_num_t, tag_queue*> m_;
};

} // namespace ibv
} // namespace mgcom

