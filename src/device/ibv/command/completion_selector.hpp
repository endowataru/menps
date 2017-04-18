
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
    
    void remove_outstanding(const mgbase::size_t num_polled, unique_lock_type& lk)
    {
        const auto num_outstanding = num_outstanding_.fetch_sub(num_polled, mgbase::memory_order_acquire);
        if (num_outstanding == 0) {
            cv_.wait(lk);
        }
    }
    
    void notify(const mgbase::size_t num_wrs)
    {
        const auto num_outstanding = num_outstanding_.fetch_add(num_wrs, mgbase::memory_order_acquire);
        if (num_outstanding == 0) {
            ult::lock_guard<ult::mutex> lk(this->mtx_);
            cv_.notify_one();
        }
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

