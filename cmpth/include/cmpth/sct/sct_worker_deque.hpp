
#pragma once

#include <cmpth/fdn.hpp>
#include <deque>

namespace cmpth {

// TODO: Provide a better (lock-free) implementation

template <typename P>
class sct_worker_deque
{
    using continuation_type = typename P::continuation_type;
    using spinlock_type = typename P::spinlock_type;
    
public:
    void local_push_top(continuation_type cont) {
        CMPTH_P_ASSERT(P, cont);
        CMPTH_P_ASSERT(P, !cont.is_root());
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        this->q_.push_back(fdn::move(cont));
        CMPTH_P_LOG_DEBUG(P, "Calling local_pop_top().", 0);
    }
    
    void local_push_bottom(continuation_type cont) {
        CMPTH_P_ASSERT(P, cont);
        CMPTH_P_ASSERT(P, !cont.is_root());
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        this->q_.push_front(fdn::move(cont));
        CMPTH_P_LOG_DEBUG(P, "Calling local_push_bottom().", 0);
    }
    
    void remote_push_bottom(continuation_type cont) {
        CMPTH_P_ASSERT(P, cont);
        CMPTH_P_ASSERT(P, !cont.is_root());
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        this->q_.push_front(fdn::move(cont));
        CMPTH_P_LOG_DEBUG(P, "Calling remote_push_bottom().", 0);
    }
    
    continuation_type try_local_pop_top() {
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        if (this->q_.empty()) return {};
        auto cont = fdn::move(this->q_.back());
        this->q_.pop_back();
        CMPTH_P_ASSERT(P, cont);
        CMPTH_P_ASSERT(P, !cont.is_root());
        CMPTH_P_LOG_DEBUG(P, "Calling try_local_pop_top().", 0);
        return cont;
    }
    
    continuation_type try_remote_pop_bottom() {
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        if (this->q_.empty()) return {};
        auto cont = fdn::move(this->q_.front());
        this->q_.pop_front();
        CMPTH_P_ASSERT(P, cont);
        CMPTH_P_ASSERT(P, !cont.is_root());
        CMPTH_P_LOG_DEBUG(P, "Calling try_remote_pop_bottom().", 0);
        return cont;
    }
    
private:
    spinlock_type mtx_;
    std::deque<continuation_type> q_;
};

} // namespace cmpth

