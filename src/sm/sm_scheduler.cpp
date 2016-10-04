
#include "sm_worker.hpp"

#include <mgult/generic/basic_scheduler.hpp>
#include <mgult/sm.hpp>

namespace mgult {

class sm_scheduler;

struct sm_scheduler_traits
{
    typedef sm_scheduler    derived_type;
    typedef scheduler       scheduler_base_type;
    typedef sm_worker       worker_type;
    typedef worker_rank_t   worker_rank_type;
    typedef ult_id          ult_id_type;
};

class sm_scheduler
    : public basic_scheduler<sm_scheduler_traits>
{
    typedef basic_scheduler<sm_scheduler_traits>    base;
    
public:
    sm_scheduler()
    {
        instance_ = this;
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
    virtual void loop(const loop_func_t func) MGBASE_OVERRIDE
    {
        base::loop_workers(num_ranks_, func);
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
    
    ult_ptr_ref try_steal_from_another(sm_worker& w)
    {
        MGBASE_ASSERT(&sm_worker::get_current_worker() == &w);
        
        const auto num_ranks = get_num_ranks();
        
        if (num_ranks == 1)
            return ult_ptr_ref{};
        
        const auto current_rank = w.get_rank();
        
        // TODO: better algorithm
        const auto random_val = static_cast<worker_rank_t>(std::rand());
        
        const auto stolen_rank = (current_rank + random_val % (num_ranks - 1) + 1) % num_ranks;
        
        MGBASE_ASSERT(stolen_rank != current_rank);
        MGBASE_ASSERT(stolen_rank < num_ranks);
        
        auto& stolen_wk = this->get_worker_of_rank(stolen_rank);
        
        return stolen_wk.try_steal();
    }
    
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

sm_scheduler* sm_scheduler::instance_; // FIXME

bool sm_worker::finished()
{
    return sched_.finished();
}

ult_ptr_ref sm_worker::try_steal_from_another()
{
    return sched_.try_steal_from_another(*this);
}


mgbase::unique_ptr<scheduler> make_sm_scheduler()
{
    return mgbase::make_unique<sm_scheduler>();
}

} // namespace mgult

