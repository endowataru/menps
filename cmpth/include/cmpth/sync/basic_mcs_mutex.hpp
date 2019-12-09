
#pragma once

#include <cmpth/fdn.hpp>

//#define CMPTH_MCS_MUTEX_USE_NEXT_UV

namespace cmpth {

template <typename P>
class basic_mcs_mutex
{
    using mcs_core_type = typename P::mcs_core_type;
    using mcs_node_type = typename P::mcs_node_type;
    
    using worker_type = typename P::worker_type;
    using suspended_thread_type = typename P::suspended_thread_type;
    
public:
    void lock() {
        auto& wk = worker_type::get_cur_worker();
        this->lock(wk);
    }
    void unlock() {
        auto& wk = worker_type::get_cur_worker();
        this->unlock(wk);
    }
    void unlock_and_wait(suspended_thread_type& saved_sth) {
        auto& wk = worker_type::get_cur_worker();
        this->unlock_and_wait(wk, saved_sth);
    }
    
    void lock(worker_type& wk)
    {
        auto& pool = P::get_node_pool();
        //const auto wk_num = wk.get_worker_num();
        const auto cur = pool.allocate(); // TODO
        
        const auto prev = this->core_.start_lock(cur);
        
        if (prev != nullptr)
        {
            #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
            cur->sth.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
            #else
            prev->sth.template wait_with<
                basic_mcs_mutex::on_lock
            >(wk, prev, cur);
            #endif
            
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
        
        #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
        //next->sth.notify(wk);
        next->sth.enter(wk);
        #else
        //cur->sth.notify(wk);
        cur->sth.enter(wk);
        #endif
        
        this->deallocate(cur);
    }
    
    void unlock_and_wait(worker_type& wk, suspended_thread_type& saved_sth)
    {
        const auto cur = this->core_.get_head();
        
        if (CMPTH_LIKELY(
            this->core_.is_unlockable(cur)
        )) {
            bool is_unlocked = true;
            
            saved_sth.template wait_with<
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
        
        #ifdef CMPTH_MCS_MUTEX_USE_NEXT_UV
        // Save the current context to saved_sth and switch to next->sth.
        saved_sth.swap(wk, next->sth);
        #else
        // Save the current context to saved_sth and switch to cur->sth.
        saved_sth.swap(wk, cur->sth);
        #endif
        
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

