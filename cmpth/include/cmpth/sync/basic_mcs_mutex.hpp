
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

#define CMPTH_MCS_MUTEX_USE_NEXT_UV

template <typename P>
class basic_mcs_mutex
{
    using mcs_core_type = typename P::mcs_core_type;
    using mcs_node_type = typename P::mcs_node_type;
    
    using worker_type = typename P::worker_type;
    using uncond_var_type = typename P::uncond_var_type;
    
public:
    void lock(worker_type& wk, mcs_node_type* const cur)
    {
        const auto prev = this->core_.start_lock(cur);
        
        if (prev != nullptr)
        {
            #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
            cur->uv.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
            #else
            prev->uv.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
            #endif
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
    mcs_node_type* unlock(worker_type& wk)
    {
        const auto cur = this->core_.get_head();
        
        if (CMPTH_LIKELY(this->core_.try_unlock(cur))) {
            return cur;
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = this->core_.try_follow_head(cur);
        }
        while (next == nullptr);
        
        #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
        next->uv.notify(wk);
        #else
        cur->uv.notify(wk);
        #endif
        
        return cur;
    }
    
    mcs_node_type* unlock_and_wait(worker_type& wk, uncond_var_type& saved_uv)
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
            
            if (CMPTH_LIKELY(is_unlocked)) { return cur; }
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = this->core_.try_follow_head(cur);
        }
        while (next == nullptr);
        
        #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
        // Save the current context to saved_uv and switch to next->uv.
        saved_uv.swap(wk, next->uv);
        #else
        // Save the current context to saved_uv and switch to cur-uv.
        saved_uv.swap(wk, cur->uv);
        #endif
        
        return cur;
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
    
    mcs_core_type core_;
};

} // namespace cmpth

