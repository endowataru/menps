
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_cv
{
    using mcs_mutex_type = typename P::mcs_mutex_type;
    using mcs_unique_lock_type = typename P::mcs_unique_lock_type;
    
    using worker_type = typename P::worker_type;
    using suspended_thread_type = typename P::suspended_thread_type;
    
    struct wait_entry {
        suspended_thread_type   sth;
        wait_entry*             next;
    };
    
public:
    void wait(mcs_unique_lock_type& lk)
    {
        wait_entry e;
        
        CMPTH_P_ASSERT(P, lk.owns_lock());
        
        e.next = this->waiting_;
        this->waiting_ = &e;
        
        const auto mtx = lk.release();
        
        mtx->unlock_and_wait(e.sth);
        
        // Lock again here.
        lk = mcs_unique_lock_type{*mtx};
    }
    
    template <typename Pred>
    void wait(mcs_unique_lock_type& lk, Pred pred)
    {
        while (!pred()) {
            this->wait(lk);
        }
    }
    
    void notify_one()
    {
        auto& wk = worker_type::get_cur_worker();
        if (const auto we = this->waiting_) {
            we->sth.notify(wk);
            this->waiting_ = we->next;
        }
    }
    
    void notify_all()
    {
        auto& wk = worker_type::get_cur_worker();
        auto we = this->waiting_;
        while (we != nullptr) {
            we->sth.notify(wk);
            we = we->next;
        }
        this->waiting_ = nullptr;
    }
    
private:
    wait_entry* waiting_ = nullptr;
};

} // namespace cmpth

