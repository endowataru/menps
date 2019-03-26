
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

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
            prev->uv.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
        }
    }
    
private:
    struct on_lock {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&            /*wk*/
        ,   mcs_node_type* const    cur
        ,   mcs_node_type* const    next
        ) {
            mcs_core_type::set_next(cur, next);
            
            return true;
        }
    };
    
public:
    void unlock(worker_type& wk, mcs_node_type* const cur)
    {
        if (CMPTH_LIKELY(this->core_.try_unlock(cur))) {
            return;
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = mcs_core_type::load_next(cur);
        }
        while (next == nullptr);
        
        cur->uv.notify(wk);
    }
    
    void unlock_and_wait(worker_type& wk, mcs_node_type* const cur, uncond_var_type& saved_uv)
    {
        if (CMPTH_LIKELY(
            this->core_.is_unlockable(cur)
        )) {
            bool is_unlocked = true;
            
            saved_uv.template wait_with<
                basic_mcs_mutex::on_unlock_and_wait
            >
            (wk, this, cur, &is_unlocked);
            
            if (CMPTH_LIKELY(is_unlocked)) { return; }
        }
        
        mcs_node_type* next = nullptr;
        do {
            next = mcs_core_type::load_next(cur);
        }
        while (next == nullptr);
        
        // Save the current context to saved_uv and switch to cur-uv.
        saved_uv.swap(wk, cur->uv);
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

