
#include "dist_scheduler.hpp"
#include "dist_worker.hpp"
#include <menps/meult/generic/basic_scheduler.hpp>
#include "global_ult_desc_pool.hpp"
#include <menps/mecom/rpc.hpp>
#include <menps/mecom/collective.hpp>
#include "dist_exit_status.hpp"

namespace menps {
namespace meth {

class dist_scheduler;

struct dist_scheduler_traits
{
    typedef dist_scheduler      derived_type;
    typedef root_scheduler      scheduler_base_type;
    typedef dist_worker         worker_type;
    typedef worker_rank_t       worker_rank_type;
    typedef ult_id              ult_id_type;
    typedef meth::base_ult::thread  worker_thread_type;
};

class dist_scheduler
    : public meult::basic_scheduler<dist_scheduler_traits>
{
    typedef meult::basic_scheduler<dist_scheduler_traits>   base;
    
public:
    explicit dist_scheduler(const dist_scheduler_config& conf)
        : desc_pool_({ conf.space, conf.stack_segment_ptr })
        , dsm_(conf.space)
    {
        instance_ = this;
        
        mecom::rpc::register_handler2(
            mecom::rpc::requester::get_instance()
        ,   steal_handler{}
        );
        mecom::rpc::register_handler2(
            mecom::rpc::requester::get_instance()
        ,   write_barrier_handler{}
        );
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
    ~dist_scheduler()
    {
        #ifdef METH_ENABLE_ASYNC_WRITE_BACK
        // Flush all ongoing operations to destroy resources
        // which are accessed concurrently by threads in DSM.
        dsm_.write_barrier();
        dsm_.read_barrier();
        #endif
    }
    
    virtual void loop(const loop_func_type& func) MEFDN_OVERRIDE
    {
        base::loop_workers(num_ranks_, func, &mecom::collective::barrier);
    }
    
    inline global_ult_ref allocate_ult()
    {
        return desc_pool_.allocate_ult();
    }
    
    inline void deallocate_ult(global_ult_ref&& th)
    {
        desc_pool_.deallocate_ult(mefdn::move(th));
    }
    
    global_ult_ref get_ult_ref_from_id(const ult_id& id)
    {
        return desc_pool_.get_ult_ref_from_id(id);
    }
    
    global_ult_ref try_steal_from_another(dist_worker& w)
    {
        const auto current_proc = mecom::current_process_id();
        const auto num_procs = mecom::number_of_processes();
        
        // TODO: better algorithm
        const auto random_val = static_cast<mecom::process_id_t>(std::rand());
        const auto random_val2 = static_cast<mecom::process_id_t>(std::rand());
        
        const auto stolen_proc =
            random_val % num_procs;
            //%(current_proc + random_val % (num_procs - 1) + 1) % num_procs;
        
        const auto stolen_wk =
            random_val2 % num_ranks_;
        
        const steal_argument arg{ stolen_wk };
        
        auto stolen = meult::make_invalid_ult_id();
        
        {
            const auto rply_msg =
                mecom::rpc::call<steal_handler>(
                    mecom::rpc::requester::get_instance()
                ,   stolen_proc
                ,   arg
                );
            
            stolen = *rply_msg;
        }
        
        if (is_invalid_ult_id(stolen)) {
            return {};
        }
        else {
            // Flush here.
            dsm_.read_barrier();
            
            return this->get_ult_ref_from_id(stolen);
        }
    }
    
private:
    struct steal_argument {
        worker_rank_t   wk_rank;
    };
    
    struct steal_handler
    {
        static const mecom::rpc::handler_id_t handler_id = 1000; // TODO
        
        typedef steal_argument  request_type;
        typedef ult_id          reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            const auto src_proc = sc.src_proc();
            
            const auto wk_rank = rqst.wk_rank;
            
            MEFDN_LOG_VERBOSE(
                "msg:Try stealing from this process..."
            );
            
            auto& wk = instance_->get_worker_of_rank(wk_rank);
            
            auto th = wk.try_steal();
            
            const auto id = th.get_id();
            
            if (is_invalid_ult_id(id)) {
                MEFDN_LOG_DEBUG(
                    "msg:Stealing failed (no queued thread).\t"
                    "theif_proc:{}\t"
                    "id:0x{:x}"
                ,   src_proc
                ,   reinterpret_cast<mefdn::uintptr_t>(id.ptr)
                );
            }
            else {
                MEFDN_LOG_INFO(
                    "msg:Stealing succeeded.\t"
                    "theif_proc:{}\t"
                    "id:0x{:x}"
                ,   src_proc
                ,   reinterpret_cast<mefdn::uintptr_t>(id.ptr)
                );
                
                #ifndef METH_ENABLE_ASYNC_WRITE_BACK
                // Reconcile here.
                instance_->dsm_.write_barrier();
                #endif
            }
            
            auto rply = sc.make_reply();
            *rply = id;
            return rply;
        }
    };
    
public:
    void do_write_barrier_at(const mecom::process_id_t proc, const ult_id th_id)
    {
        mecom::rpc::call<write_barrier_handler>(
            mecom::rpc::requester::get_instance()
        ,   proc
        ,   write_barrier_request{ th_id }
        );
    }
    
private:
    struct write_barrier_request {
        ult_id th_id;
    };
    
    struct write_barrier_handler
    {
        static const mecom::rpc::handler_id_t handler_id = 1001;
        
        typedef write_barrier_request       request_type;
        typedef void                        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            instance_->dsm_.write_barrier();
            
            auto& rqst = sc.request();
            
            MEFDN_LOG_INFO(
                "msg:Done write barrier.\t"
                "th_id:0x{:x}"
            ,   reinterpret_cast<mefdn::uintptr_t>(rqst.th_id.ptr)
            );
            
            return sc.make_reply();
        }
    };
    
public:
    bool finished() {
        return exit_st_.is_ready();
    }
    
    void set_started()
    {
        exit_st_.clear();
    }
    void set_finished()
    {
        exit_st_.set_finished(0); // TODO: pass exit code
    }
    
    static dist_worker& get_current_worker() {
        return dist_worker::get_current_worker();
    }
    
    static dist_scheduler& get_current_scheduler() {
        return *instance_;
    }
    
    void global_barrier()
    {
        mecom::collective::barrier();
    }
    
    global_ult_desc_pool& get_desc_pool() { // TODO: will be removed
        return desc_pool_;
    }
    
    medsm::space_ref& get_dsm() const noexcept {
        return dsm_;
    }
    
private:
    static worker_rank_t get_num_ranks_from_env()
    {
        const auto s = getenv("METH_NUM_WORKERS");
        
        if (s != nullptr) {
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
    
    medsm::space_ref&       dsm_;
    
    dist_exit_status        exit_st_;
    
    static dist_scheduler* instance_; // TODO: singleton
};

dist_scheduler* dist_scheduler::instance_ = nullptr;

dist_scheduler_ptr make_dist_scheduler(const dist_scheduler_config& conf)
{
    return mefdn::make_unique<dist_scheduler>(conf);
}


dist_worker::dist_worker(dist_scheduler& sched, const worker_rank_t rank)
    : base({ 1024 * 1024 }) // TODO
    , sched_(sched)
    , rank_(rank)
    , join_stack_area_(mefdn::make_unique<mefdn::uint8_t []>(join_stack_size))
    { }

global_ult_ref dist_worker::allocate_ult()
{
    return sched_.allocate_ult();
}

void dist_worker::deallocate_ult(global_ult_ref&& th)
{
    sched_.deallocate_ult(mefdn::move(th));
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

void dist_worker::on_before_switch(global_ult_ref& /*from_th*/, global_ult_ref& to_th)
{
    if (!to_th.is_valid()) {
        return;
    }
    
    // Set the owner.
    // TODO: to_th is not locked.
    to_th.set_owner_proc(mecom::current_process_id());
    
    auto& dsm = sched_.get_dsm();
    
    const auto stack_ptr = to_th.get_stack_ptr();
    const auto stack_size = to_th.get_stack_size();
    
    // TODO: portable way for processors with upward call stacks
    const auto stack_first_ptr = static_cast<mefdn::uint8_t*>(stack_ptr) - stack_size;
    
    MEFDN_LOG_DEBUG(
        "msg:Pinning call stack before switching to different thread.\t"
        "stack_ptr:{}\t"
        "stack_size:{}"
    ,   reinterpret_cast<mefdn::uintptr_t>(stack_ptr)
    ,   stack_size
    );
    
    dsm.pin(stack_first_ptr, stack_size);
}
template <bool IsFromLocked>
void dist_worker::on_after_switch(global_ult_ref& from_th, global_ult_ref& /*to_th*/)
{
    if (!from_th.is_valid()) {
        return;
    }
    
    auto& dsm = sched_.get_dsm();
    
    const auto stack_ptr = from_th.get_stack_ptr();
    const auto stack_size = from_th.get_stack_size();
    
    // TODO: portable way for processors with upward call stacks
    const auto stack_first_ptr = static_cast<mefdn::uint8_t*>(stack_ptr) - stack_size;
    
    MEFDN_LOG_DEBUG(
        "msg:Unpinning call stack after switching to different thread.\t"
        "stack_ptr:{}\t"
        "stack_size:{}"
    ,   reinterpret_cast<mefdn::uintptr_t>(stack_ptr)
    ,   stack_size
    );
    
    dsm.unpin(stack_first_ptr, stack_size);
    
    /*
    // TODO: Check when METH_ENABLE_RELAXED_ULT_DESC is enabled
    
    auto lk = 
        IsFromLocked
        ? from_th.get_lock(mefdn::adopt_lock)
        : from_th.get_lock();
    
    // Set the owner.
    from_th.set_owner_proc(lk, mecom::current_process_id());
    
    if (IsFromLocked) {
        // This function doesn't unlock.
        // The worker will unlock it later.
        lk.release();
    }
    */
    
    #ifdef METH_ENABLE_ASYNC_WRITE_BACK
    // Start write barrier.
    dsm.async_write_barrier(
        from_th.make_update_stamp(lk, this->sched_.get_desc_pool())
    );
    #endif
}

#ifdef METH_ENABLE_ASYNC_WRITE_BACK
namespace /*unnamed*/ {

inline void wait_for_latest_stamp(global_ult_ref& th, global_ult_ref::unique_lock_type& lk)
{
    while (!th.is_latest_stamp(lk)) {
        lk.unlock();
        base_ult::this_thread::yield();
        lk.lock();
    }
}

} // unnamed namespace
#endif

struct dist_worker::join_already_data
{
    dist_worker&    self;
    void*           stack_ptr;
    mefdn::size_t  stack_size;
    
    MEFDN_NORETURN
    static void handler(const meult::fcontext_argument<void, join_already_data> arg)
    {
        auto& d = *arg.data;
        auto& self = d.self;
        
        auto& dsm = self.sched_.get_dsm();
        
        dsm.unpin(d.stack_ptr, d.stack_size);
        
        dsm.read_barrier();
        
        dsm.pin(d.stack_ptr, d.stack_size);
        
        void* const null = nullptr;
        meult::jump_fcontext<void>(arg.fctx, null);
    }
};

void dist_worker::on_join_already(global_ult_ref& current_th, global_ult_ref& joinee_th, global_ult_ref::unique_lock_type& lk)
{
    const auto owner_proc = joinee_th.get_owner_proc();
    
    MEFDN_ASSERT(mecom::valid_process_id(owner_proc));
    
    if (owner_proc != mecom::current_process_id())
    {
        #ifdef METH_ENABLE_ASYNC_WRITE_BACK
        wait_for_latest_stamp(joinee_th, lk);
        #else
        sched_.do_write_barrier_at(owner_proc, joinee_th.get_id());
        #endif
        
        // Do read barrier on a different stack to unpin the current stack.
        
        const auto stack_ptr = current_th.get_stack_ptr();
        const auto stack_size = current_th.get_stack_size();
        
        join_already_data data{
            *this
        ,   static_cast<mefdn::uint8_t*>(current_th.get_stack_ptr()) - stack_size
        ,   stack_size
        };
        
        const auto ctx =
            meult::make_fcontext(
                join_stack_area_.get() + join_stack_size
            ,   join_stack_size
            ,   &join_already_data::handler
            );
        
        meult::jump_fcontext<void>(ctx, &data);
    }
}
void dist_worker::on_join_resume(global_ult_ref&& child_th)
{
    #ifdef METH_ENABLE_ASYNC_WRITE_BACK
    // Lock the thread.
    // This is different from shared-memory implementation.
    auto lk = child_th.get_lock();
    
    // If the write back is completed, the thread is destroyed.
    // Note: We don't lock the child thread.
    if (child_th.is_latest_stamp(lk)) {
        // Unlock the thread first to destroy it.
        lk.unlock();
        
        // Destroy the thread descriptor of the child thread.
        //
        // Both shared-memory and distributed-memory versions destroy the child thread
        // because the write back is already completed
        // (and the current thread has already been reading the changes).
        this->deallocate_ult( mefdn::move(child_th) );
    }
    else {
        // Detach the thread.
        // This execution path is disabled in shared-memory implementation.
        child_th.set_detached(lk);
    }
    
    // Unlock the child thread.
    
    #else
    
    this->deallocate_ult( mefdn::move(child_th) );
    
    #endif
}
void dist_worker::on_exit_resume(global_ult_ref& th)
{
    #ifdef METH_ENABLE_ASYNC_WRITE_BACK
    auto lk = th.get_lock();
    #endif
    
    const auto owner_proc = th.get_owner_proc();
    
    MEFDN_ASSERT(mecom::valid_process_id(owner_proc));
    
    if (owner_proc != mecom::current_process_id())
    {
        #ifdef METH_ENABLE_ASYNC_WRITE_BACK
        wait_for_latest_stamp(th, lk);
        #else
        sched_.do_write_barrier_at(owner_proc, th.get_id());
        #endif
        
        // Do a read barrier directly on the current stack.
        // The current stack is deallocated
        // after switching to the continuation.
        sched_.get_dsm().read_barrier();
    }
}

void dist_worker::initialize_on_this_thread()
{
    stack_area_.reset(
        new mefdn::uint8_t[page_fault_stack_size]
    );
    alter_stack_ = 
        mefdn::make_unique<alternate_signal_stack>( 
            stack_area_.get()
        ,   page_fault_stack_size
        );
    
    tls_base::initialize_on_this_thread();
    
    auto& dsm = sched_.get_dsm();
    dsm.enable_on_this_thread();
}

void dist_worker::finalize_on_this_thread()
{
    auto& dsm = sched_.get_dsm();
    dsm.disable_on_this_thread();
    
    tls_base::finalize_on_this_thread();
    
    alter_stack_.reset();
    stack_area_.reset();
}

const mefdn::size_t dist_worker::page_fault_stack_size;

} // namespace meth
} // namespace menps

