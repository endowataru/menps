
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_mutex
{
    using mcs_core_type = typename P::mcs_core_type;
    using mcs_node_type = typename P::mcs_node_type;
    
    using worker_type = typename P::worker_type;
    using uncond_var_type = typename P::uncond_var_type;
    
public:
    void lock() {
        auto& wk = worker_type::get_cur_worker();
        this->lock(wk);
    }
    void unlock() {
        auto& wk = worker_type::get_cur_worker();
        this->unlock(wk);
    }
    void unlock_and_wait(uncond_var_type& saved_uv) {
        auto& wk = worker_type::get_cur_worker();
        this->unlock_and_wait(wk, saved_uv);
    }
    
    void lock(worker_type& wk)
    {
        auto& pool = P::get_node_pool();
        //const auto wk_num = wk.get_worker_num();
        const auto cur = pool.allocate(); // TODO
        
        const auto prev = this->core_.start_lock(cur);
        
        if (prev != nullptr)
        {
            prev->uv.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
            
            CMPTH_P_ASSERT(P, this->core_.get_head() == cur);
        }
    }
    
private:
    struct on_lock {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&            /*wk*/
        ,   mcs_node_type* const    prev
        ,   mcs_node_type* const    cur
        ) {
            mcs_core_type::set_next(prev, cur);
            
            return true;
        }
    };
    
public:
    void unlock(worker_type& wk)
    {
        const auto cur = this->core_.get_head();
        
        if (CMPTH_LIKELY(this->core_.try_unlock(cur))) {
            this->deallocate(cur);
            return;
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = this->core_.try_follow_head(cur);
        }
        while (next == nullptr);
        
        cur->uv.notify(wk);
        
        this->deallocate(cur);
    }
    
    void unlock_and_wait(worker_type& wk, uncond_var_type& saved_uv)
    {
        const auto cur = this->core_.get_head();
        
        if (CMPTH_LIKELY(
            this->core_.is_unlockable(cur)
        )) {
            bool is_unlocked = true;
            
            saved_uv.template wait_with<
                basic_mcs_mutex::on_unlock_and_wait
            >
            (wk, this, cur, &is_unlocked);
            
            if (CMPTH_LIKELY(is_unlocked)) {
                this->deallocate(cur);
                return;
            }
            
            CMPTH_P_ASSERT(P, this->core_.get_head() == cur);
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = this->core_.try_follow_head(cur);
        }
        while (next == nullptr);
        
        // Save the current context to saved_uv and switch to cur->uv.
        saved_uv.swap(wk, cur->uv);
        
        this->deallocate(cur);
    }
    
private:
    struct on_unlock_and_wait {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&            /*wk*/
        ,   basic_mcs_mutex* const  self
        ,   mcs_node_type* const    cur
        ,   bool* const             is_unlocked
        ) {
            if (CMPTH_LIKELY(self->core_.try_unlock(cur))) {
                return true;
            }
            else {
                *is_unlocked = false;
                return false;
            }
        }
    };
    
    static void deallocate(mcs_node_type* const cur) {
        // Important: Renew the worker because it may change.
        /*auto& wk2 = worker_type::get_cur_worker();
        const auto wk_num = wk2.get_worker_num();*/
        
        auto& pool = P::get_node_pool();
        pool.deallocate(/*wk_num, */cur); // TODO
    }
    
    mcs_core_type core_;
};

} // namespace cmpth

