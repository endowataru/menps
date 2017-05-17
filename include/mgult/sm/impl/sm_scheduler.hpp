
#pragma once

#include "sm_worker.hpp"

#include <mgult/generic/basic_scheduler.hpp>
#include <mgult/root_scheduler.hpp>

namespace mgult {
namespace sm {

class sm_scheduler;

struct sm_scheduler_traits
{
    typedef sm_scheduler    derived_type;
    typedef root_scheduler  scheduler_base_type;
    typedef sm_worker       worker_type;
    typedef worker_rank_t   worker_rank_type;
    typedef ult_id          ult_id_type;
};

class sm_scheduler
    : public basic_scheduler<sm_scheduler_traits>
{
    typedef basic_scheduler<sm_scheduler_traits>    base;
    
public:
    sm_scheduler();
    
private:
    // GCC 4.4 requires template parameters are in external linkage.
    struct empty_barrier {
        void operator() () {
            // do nothing
        }
    };
    
public:
    virtual void loop(const mgbase::function<void ()>& func) MGBASE_OVERRIDE
    {
        base::loop_workers(num_ranks_, func, empty_barrier{});
    }
    
    void set_started()
    {
        finished_.store(false, mgbase::memory_order_relaxed);
    }
    void set_finished()
    {
        finished_.store(true, mgbase::memory_order_release);
    }
    
    bool finished() const MGBASE_NOEXCEPT
    {
        return finished_.load(mgbase::memory_order_acquire);
    }
    
    ult_ptr_ref try_steal_from_another(sm_worker& w);
    
    static sm_worker& get_current_worker() {
        return sm_worker::get_current_worker();
    }
    
    static sm_scheduler& get_current_scheduler() {
        return *instance_;
    }
    
private:
    worker_rank_t get_num_ranks() const MGBASE_NOEXCEPT {
        return num_ranks_;
    }
    
    static worker_rank_t get_num_ranks_from_env()
    {
        const auto s = getenv("MGULT_NUM_WORKERS");
        
        if (s != MGBASE_NULLPTR) {
            auto num_ranks = static_cast<worker_rank_t>(atoi(s));
            if (num_ranks == 0) {
                // TODO: Should it be an exception?
                return 1; // Default
            }
            return num_ranks;
        }
        else
            return 1; // Default
    }
    
    mgbase::atomic<bool> finished_;
    
    worker_rank_t num_ranks_;
    
    static sm_scheduler* instance_; // TODO: singleton
};

} // namespace sm
} // namespace mgult

