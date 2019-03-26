
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class basic_pool_mcs_mutex
{
    using mcs_mutex_type = typename P::mcs_mutex_type;
    using mcs_node_type = typename P::mcs_node_type;
    
    using worker_type = typename P::worker_type;
    using uncond_var_type = typename P::uncond_var_type;
    
public:
    void lock()
    {
        auto& wk = worker_type::get_cur_worker();
        
        auto& pool = P::get_node_pool();
        const auto cur = pool.allocate(wk);
        
        this->mtx_.lock(wk, cur);
        
        this->cur_ = cur;
    }
    
    void unlock()
    {
        const auto cur = this->cur_;
        this->cur_ = nullptr;
        
        auto& wk = worker_type::get_cur_worker();
        this->mtx_.unlock(wk, cur);
        
        auto& wk2 = worker_type::get_cur_worker();
        
        auto& pool = P::get_node_pool();
        pool.deallocate(wk2, cur);
    }
    
    void unlock_and_wait(uncond_var_type& saved_uv)
    {
        const auto cur = this->cur_;
        this->cur_ = nullptr;
        
        auto& wk = worker_type::get_cur_worker();
        this->mtx_.unlock_and_wait(wk, cur, saved_uv);
        
        auto& wk2 = worker_type::get_cur_worker();
        
        auto& pool = P::get_node_pool();
        pool.deallocate(wk2, cur);
    }
    
private:
    mcs_mutex_type  mtx_;
    fdn::byte       pad1_[CMPTH_CACHE_LINE_SIZE - sizeof(mcs_mutex_type)];
    
    mcs_node_type*  cur_;
    fdn::byte       pad2_[CMPTH_CACHE_LINE_SIZE - sizeof(mcs_node_type*)];
};

} // namespace cmpth

