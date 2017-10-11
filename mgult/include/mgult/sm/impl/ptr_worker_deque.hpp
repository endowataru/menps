
#pragma once

#include "ult_ptr_ref.hpp"

namespace mgult {
namespace sm {

struct ptr_worker_deque_conf
{
    mgbase::size_t deque_size;
};

template <typename Deque>
class ptr_worker_deque
{
    typedef ult_ptr_ref ult_ref_type;
    
public:
    explicit ptr_worker_deque(const ptr_worker_deque_conf& conf)
        : dq_(conf.deque_size) { }
    
    void push_top(ult_ref_type&& th)
    {
        dq_.push_top(th.get_id());
    }
    
    void push_bottom(ult_ref_type&& th)
    {
        dq_.push_bottom(th.get_id());
    }
    
    ult_ref_type try_pop_top()
    {
        return ult_ref_type{ dq_.try_pop_top() };
    }
    
    ult_ref_type try_pop_bottom()
    {
        return ult_ref_type{ dq_.try_pop_bottom() };
    }
    
private:
    Deque dq_;
};

} // namespace sm
} // namespace mgult

