
#include "dist_scheduler.hpp"
#include "dist_worker.hpp"
#include <mgult/generic/basic_scheduler.hpp>
#include "global_ult_desc_pool.hpp"
#include <mgcom/rpc.hpp>

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
        
        mgcom::rpc::register_handler<steal_handler>();
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
    virtual void loop(const loop_func_t func) MGBASE_OVERRIDE
    {
        if (mgcom::current_process_id() == 0) {
            // Add the main thread.
            base::loop_workers(num_ranks_, func);
        }
        else {
            // Start as normal workers.
            base::loop_workers(num_ranks_, MGBASE_NULLPTR);
        }
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
    
    global_ult_ref try_steal_from_another(dist_worker& w)
    {
        const auto current_proc = mgcom::current_process_id();
        const auto num_procs = mgcom::number_of_processes();
        
        // TODO: better algorithm
        const auto random_val = static_cast<mgcom::process_id_t>(std::rand());
        const auto random_val2 = static_cast<mgcom::process_id_t>(std::rand());
        
        const auto stolen_proc =
            random_val % num_procs;
            //%(current_proc + random_val % (num_procs - 1) + 1) % num_procs;
        
        const auto stolen_wk =
            random_val2 % num_ranks_;
        
        const steal_argument arg{ stolen_wk };
        
        auto stolen = mgult::make_invalid_ult_id();
        
        mgcom::rpc::remote_call<steal_handler>(
            stolen_proc
        ,   arg
        ,   &stolen
        );
        
        if (is_invalid_ult_id(stolen)) {
            return {};
        }
        else {
            return this->get_ult_ref_from_id(stolen);
        }
    }
    
private:
    struct steal_argument {
        worker_rank_t   wk_rank;
    };
    
    struct steal_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = 500; // TODO
        
        typedef steal_argument  argument_type;
        typedef ult_id          return_type;
        
        static return_type on_request(
            const mgcom::rpc::handler_parameters&   params
        ,   const argument_type&                    arg
        ) {
            const auto wk_rank = arg.wk_rank;
            
            MGBASE_LOG_VERBOSE(
                "msg:Try stealing from this process..."
            );
            
            auto& wk = instance_->get_worker_of_rank(wk_rank);
            
            auto th = wk.try_steal();
            
            const auto id = th.get_id();
            
            MGBASE_LOG_VERBOSE(
                "msg:Stolen thread.\t"
                "id:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(id.ptr)
            );
            
            return id;
        }
    };
    
public:
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

global_ult_ref dist_worker::try_steal_from_another()
{
    return sched_.try_steal_from_another(*this);
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

