
#pragma once

#include <cmpth/fdn.hpp>

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
    }
    
    void unlock()
    {
        auto& wk = worker_type::get_cur_worker();
        const auto cur = this->mtx_.unlock(wk);
        
        // Important: Renew the worker because it may change.
        auto& wk2 = worker_type::get_cur_worker();
        
        auto& pool = P::get_node_pool();
        pool.deallocate(wk2, cur);
    }
    
    void unlock_and_wait(uncond_var_type& saved_uv)
    {
        auto& wk = worker_type::get_cur_worker();
        const auto cur = this->mtx_.unlock_and_wait(wk, saved_uv);
        
        // Important: Renew the worker because it may change.
        auto& wk2 = worker_type::get_cur_worker();
        
        auto& pool = P::get_node_pool();
        pool.deallocate(wk2, cur);
    }
    
private:
    mcs_mutex_type  mtx_;
};

} // namespace cmpth

