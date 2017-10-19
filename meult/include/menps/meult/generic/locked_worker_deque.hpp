
#pragma once

#include <menps/meult/ult_id.hpp>
#include <menps/mefdn/threading/spinlock.hpp>
#include <menps/mefdn/threading/lock_guard.hpp>
#include <deque>

namespace menps {
namespace meult {

template <typename Ref>
class locked_worker_deque
{
    typedef mefdn::spinlock    lock_type;
    typedef Ref                 ult_ref_type;
    
public:
    void push_top(ult_ref_type&& th)
    {
        mefdn::lock_guard<lock_type> lc(lock_);
        
        deq_.push_back(th.get_id());
    }
    
    void push_bottom(ult_ref_type&& th)
    {
        mefdn::lock_guard<lock_type> lc(lock_);
        
        deq_.push_front(th.get_id());
    }
    
    ult_ref_type try_pop_top()
    {
        mefdn::lock_guard<lock_type> lc(lock_);
        
        if (deq_.empty()) {
            return {};
        }
        else {
            auto id = deq_.back();
            deq_.pop_back();
            return ult_ref_type(id);
        }
    }
    
    ult_ref_type try_pop_bottom()
    {
        mefdn::lock_guard<lock_type> lc(lock_);
        
        if (deq_.empty()) {
            return {};
        }
        else {
            auto id = deq_.front();
            deq_.pop_front();
            return ult_ref_type(id);
        }
    }
    
private:
    lock_type lock_;
    std::deque<ult_id> deq_;
};

} // namespace meult
} // namespace menps

