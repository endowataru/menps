
#include "dist_scheduler.hpp"
#include "dist_worker.hpp"
#include <mgult/generic/basic_scheduler.hpp>
#include "global_ult_desc_pool.hpp"

namespace mgth {

class dist_scheduler;

struct dist_scheduler_traits
{
    typedef dist_scheduler      derived_type;
    typedef scheduler           scheduler_base_type;
    typedef dist_worker         worker_type;
    typedef worker_rank_t       worker_rank_type;
    typedef ult_id              ult_id_type;
};

class dist_scheduler
    : public mgult::basic_scheduler<dist_scheduler_traits>
{
    typedef mgult::basic_scheduler<dist_scheduler_traits>   base;
    
public:
    explicit dist_scheduler(mgdsm::dsm_interface& dsm)
        : desc_pool_(dsm)
        , dsm_(dsm)
    {
        instance_ = this;
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
    virtual void loop(const loop_func_t func) MGBASE_OVERRIDE
    {
        base::loop_workers(num_ranks_, func);
    }
    
    inline global_ult_ref allocate_ult()
    {
        return desc_pool_.allocate_ult();
    }
    
    inline void deallocate_ult(global_ult_ref&& th)
    {
        desc_pool_.deallocate_ult(mgbase::move(th));
    }
    
    global_ult_ref get_ult_ref_from_id(const ult_id& id)
    {
        return desc_pool_.get_ult_ref_from_id(id);
    }
    
    bool finished() {
        // FIXME
        return false;
    }
    
    void set_started() {
        // FIXME
    }
    /*{
        finished_.store(false, mgbase::memory_order_relaxed);
    }*/
    void set_finished() {
        // FIXME
    }
    /*{
        finished_.store(true, mgbase::memory_order_release);
    }*/
    
    static dist_worker& get_current_worker() {
        return dist_worker::get_current_worker();
    }
    
    static dist_scheduler& get_current_scheduler() {
        return *instance_;
    }
    
    global_ult_desc_pool& get_desc_pool() { // TODO: will be removed
        return desc_pool_;
    }
    
    mgdsm::dsm_interface& get_dsm() const MGBASE_NOEXCEPT {
        return dsm_;
    }
    
private:
    static worker_rank_t get_num_ranks_from_env()
    {
        const auto s = getenv("MGTH_NUM_WORKERS");
        
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
    
    worker_rank_t           num_ranks_;
    
    global_ult_desc_pool    desc_pool_;
    
    mgdsm::dsm_interface&   dsm_;
    
    static dist_scheduler* instance_; // TODO: singleton
};

dist_scheduler* dist_scheduler::instance_ = MGBASE_NULLPTR;

dist_scheduler_ptr make_dist_scheduler(mgdsm::dsm_interface& dsm)
{
    return mgbase::make_unique<dist_scheduler>(dsm);
}


dist_worker::dist_worker(dist_scheduler& sched, const worker_rank_t rank)
    : base({ 1024 * 1024 }) // TODO
    , sched_(sched)
    , rank_(rank)
    { }

global_ult_ref dist_worker::allocate_ult()
{
    return sched_.allocate_ult();
}

void dist_worker::deallocate_ult(global_ult_ref&& th)
{
    sched_.deallocate_ult(mgbase::move(th));
}

global_ult_ref dist_worker::get_ult_ref_from_id(const ult_id& id)
{
    return sched_.get_ult_ref_from_id(id);
}

bool dist_worker::finished()
{
    return sched_.finished();
}

void dist_worker::before_switch_to(global_ult_ref& th)
{
    auto& dsm = sched_.get_dsm();
    
    const auto stack_ptr = th.get_stack_ptr();
    const auto stack_size = th.get_stack_size();
    
    // TODO: portable way for processors with upward call stacks
    const auto stack_first_ptr = static_cast<mgbase::uint8_t*>(stack_ptr) - stack_size;
    
    dsm.fetch_writable_range(stack_first_ptr, stack_size);
}

const mgbase::size_t dist_worker::stack_size;

} // namespace mgth

