
#pragma once

#include <menps/meomp/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/medsm2/prof.hpp>

namespace menps {
namespace meomp {

template <typename P>
class child_worker_group
{
    MEFDN_DEFINE_DERIVED(P)
    
    using child_worker_type = typename P::child_worker_type;
    using worker_base_type = typename P::worker_base_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    using comm_ult_itf_type = typename P::comm_ult_itf_type;
    using thread_type = typename comm_ult_itf_type::thread;
    using barrier_type = typename comm_ult_itf_type::barrier;
    
    struct worker_info
    {
        child_worker_type   wk;
        thread_type         th;
    };
    
public:
    void start_parallel(
        const omp_func_type func
    ,   const omp_data_type data
    ,   const int           total_num_threads
    ,   const int           thread_num_first
    ,   const int           num_local_threads
    ,   const bool          is_master_node
    ) {
        auto& self = this->derived();
        
        this->thread_num_first_  = thread_num_first;
        
        const int num_child_threads = num_local_threads - (is_master_node ? 1 : 0);
        this->num_child_threads_ = num_child_threads;
        
        // The barrier is fired when "# of local threads" participates in.
        this->bar_ = mefdn::make_unique<barrier_type>(num_local_threads);
        
        // The child workers are created without the master thread on the master node.
        this->wis_ = mefdn::make_unique<worker_info []>(num_child_threads);
        
        for (int i = 0; i < num_child_threads; ++i) {
            const auto thread_num = thread_num_first + (is_master_node ? 1 : 0) + i;
            
            auto& wk = this->wis_[i].wk;
            wk.set_num_threads(total_num_threads);
            wk.set_thread_num(thread_num);
            
            this->wis_[i].th =
                thread_type([&wk, &self, func, data] {
                    wk.loop(self, func, data);
                });
        }
    }
    
    void end_parallel()
    {
        for (int i = 0; i < this->num_child_threads_; ++i) {
            this->wis_[i].th.join();
        }
        
        this->wis_.reset();
        
        this->bar_.reset();
    }
    
    // This function is called when the worker decided to do a barrier.
    void barrier_on(worker_base_type& wk)
    {
        auto& self = this->derived();
        
        using medsm2::prof;
        using medsm2::prof_kind;
        
        const auto p = prof::start();
        
        // Check whether this function is called inside a parallel region.
        if (bar_) {
            // Do a barrier in this process to complete preceding memory writes.
            this->bar_->arrive_and_wait();
            
            if (wk.get_thread_num() == thread_num_first_) {
                auto& sp = self.get_dsm_space();
                
                // Do a DSM barrier.
                sp.barrier();
            }
            
            // Do a barrier in this process again to ensure the freshness of subsequent memory loads.
            this->bar_->arrive_and_wait();
        }
        
        prof::finish(prof_kind::omp_barrier, p);
    }
    
private:
    mefdn::unique_ptr<barrier_type> bar_;
    int thread_num_first_ = 0;
    int num_child_threads_ = 0;
    mefdn::unique_ptr<worker_info []> wis_;
};

} // namespace meomp
} // namespace menps

