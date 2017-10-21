
#pragma once

#include "global_ult_ref.hpp"
#include <menps/meult/generic/default_worker_deque.hpp>
#include "global_ult_desc_pool.hpp" // TODO: remove dependency

namespace menps {
namespace meth {

class global_ult_desc_pool;

struct dist_worker_deque_conf
{
    mefdn::size_t          deque_size;
};

class dist_worker_deque
{
public:
    explicit dist_worker_deque(const dist_worker_deque_conf& conf)
        : conf_(conf)
        , dq_(conf.deque_size) { }
    
    void push_top(global_ult_ref&& th)
    {
        dq_.push_top(th.get_id());
    }
    
    void push_bottom(global_ult_ref&& th)
    {
        dq_.push_bottom(th.get_id());
    }
    
    ult_id try_pop_top()
    {
        return dq_.try_pop_top();
    }
    
    ult_id try_pop_bottom()
    {
        return dq_.try_pop_bottom();
    }
    
private:
    dist_worker_deque_conf          conf_;
    meult::default_worker_deque     dq_;
};

} // namespace meth
} // namespace menps

