
#pragma once

#include "common.hpp"
#include <mgbase/shared_ptr.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/logger.hpp>
#include <vector>

namespace mgult {

template <typename Traits>
class basic_scheduler
    : public Traits::scheduler_base_type
{
    typedef typename Traits::derived_type       derived_type;
    typedef typename Traits::worker_type        worker_type;
    typedef typename Traits::worker_rank_type   worker_rank_type;
    typedef typename Traits::ult_id_type        ult_id_type;
    
private:
    static void* main_thread_handler(void* const arg)
    {
        const auto f = reinterpret_cast<loop_func_t>(arg);
        
        f();
        
        // This thread might be stolen by other nodes in distributed work-stealing.
        // Need to renew the scheduler here.
        derived_type::get_current_scheduler().set_finished();
        
        return MGBASE_NULLPTR;
    }
    
    struct worker_loop_functor
    {
        worker_type& wk;
        
        void operator() ()
        {
            wk.initialize_on_this_thread();
            
            wk.loop();
            
            wk.finalize_on_this_thread();
        }
    };
    
    struct worker_loop_main_functor
    {
        worker_type&    wk;
        loop_func_t     func;
        
        void operator() ()
        {
            wk.initialize_on_this_thread();
            
            // FIXME: detach
            const auto t = wk.fork_parent_first(&main_thread_handler, reinterpret_cast<void*>(func));
            
            wk.detach(t);
            
            wk.loop();
            
            wk.finalize_on_this_thread();
        }
    };
    
public:
    template <typename BarrierFunc>
    void loop_workers(
        const worker_rank_type  num_ranks
    ,   const loop_func_t       func
    ,   BarrierFunc             barrier_func
    )
    {
        this->derived().set_started();
        
        this->workers_.resize(num_ranks);
        
        // Initialize workers.
        for (worker_rank_type rank = 0; rank < num_ranks; ++rank)
        {
            auto& info = this->workers_[rank];
            info.wk = mgbase::make_shared<worker_type>(this->derived(), rank);
        }
        
        // Insert a global barrier for distributed work-stealing.
        // Initialization on all the processes must be completed
        // before stealing occurs.
        barrier_func();
        
        // Push a main thread to the first worker (rank 0)
        // and start a loop in the worker.
        {
            auto& info0 = this->workers_[0];
            
            if (func != MGBASE_NULLPTR)
            {
                info0.real_th = mgbase::make_shared<mgbase::thread>(
                    mgbase::thread(worker_loop_main_functor{ *info0.wk, func })
                );
            }
            else {
                info0.real_th = mgbase::make_shared<mgbase::thread>(
                    mgbase::thread(worker_loop_functor{ *info0.wk })
                );
            }
        }
        
        // Start a loop in other workers.
        for (worker_rank_type rank = 1; rank < num_ranks; ++rank)
        {
            auto& info = this->workers_[rank];
            info.real_th = mgbase::make_shared<mgbase::thread>(
                mgbase::thread(worker_loop_functor{ *info.wk })
            );
        }
        
        MGBASE_LOG_VERBOSE(
            "msg:Started all workers."
        );
        
        // Wait for all of the workers to finish.
        
        MGBASE_RANGE_BASED_FOR(auto& info, this->workers_)
        {
            info.real_th->join();
        }
        
        MGBASE_LOG_VERBOSE(
            "msg:All workers finished."
        );
        
        this->workers_.clear();
    }
    
    virtual ult_id_type fork(const fork_func_t func, void* const arg) MGBASE_OVERRIDE
    {
        return this->derived().get_current_worker().fork_child_first(func, arg);
    }
    
    virtual void* join(const ult_id_type& id) MGBASE_OVERRIDE
    {
        return this->derived().get_current_worker().join(id);
    }
    
    virtual void detach(const ult_id_type& id) MGBASE_OVERRIDE
    {
        this->derived().get_current_worker().detach(id);
    }
    
    virtual void yield() MGBASE_OVERRIDE
    {
        this->derived().get_current_worker().yield();
    }
    
    MGBASE_NORETURN
    virtual void exit(void* const ret) MGBASE_OVERRIDE
    {
        this->derived().get_current_worker().exit(ret);
    }
    
    worker_type& get_worker_of_rank(const worker_rank_type rank)
    {
        MGBASE_ASSERT(rank < workers_.size());
        const auto& wk = workers_[rank].wk;
        MGBASE_ASSERT(wk != MGBASE_NULLPTR);
        return *wk;
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    struct worker_info {
        // TODO: Currently shared_ptr is used
        //       because old libstdc++'s vector doesn't allow move-only types as elements.
        //       They are unique in fact.
        mgbase::shared_ptr<mgbase::thread>  real_th;
        mgbase::shared_ptr<worker_type>     wk;
    };
    
    std::vector<worker_info> workers_;
};

} // namespace mgult

