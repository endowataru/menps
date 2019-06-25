
#pragma once

#include <menps/meomp/common.hpp>

//#define MEOMP_CALL_PIN_ON_WORKER_THREAD

namespace menps {
namespace meomp {

template <typename P>
class omp_worker
    : public P::single_worker_type
{
    MEFDN_DEFINE_DERIVED(P)
    
    using task_ref_type = typename P::task_ref_type;
    using call_stack_type = typename P::call_stack_type;
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    
private:
    using base = typename P::single_worker_type;
    
public:
    void set_work_stack(call_stack_type work_stk) {
        this->work_tk_ = work_stk.get_task_ref();
        base::set_work_stack(mefdn::move(work_stk));
    }
    void reset_work_stack() {
        base::unset_work_stack().release();
    }
    
    int get_thread_num() const noexcept {
        return this->thread_num_;
    }
    int get_num_threads() const noexcept {
        return this->num_threads_;
    }

    void set_thread_num(const int num) noexcept {
        this->thread_num_ = num;
    }
    void set_num_threads(const int num) noexcept {
        this->num_threads_ = num;
    }
    
private:
    template <typename Func>
    struct delegated_main
    {
        derived_type&   self;
        Func            func;
        
        void operator() ()
        {
            this->self.start_user_code_on_this_thread();
            
            this->self.execute(
                [this] {
                    this->func();
                }
            );
            
            this->self.end_user_code_on_this_thread();
        }
    };
    
public:
    template <typename Func>
    void start_worker(Func&& func)
    {
        auto& self = this->derived();
        
        // Execute this function object immediately.
        delegated_main<mefdn::decay_t<Func>>{
            self
        ,   // Copy the func so that it can survive
            // after start_worker() function exits.
            mefdn::forward<Func>(func)
        }();
    }
    void end_worker()
    {
        // Do nothing.
    }
    
    // Called on #pragma omp barrier.
    void barrier()
    {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::barrier;
        
        this->set_cmd_and_suspend(info);
    }
    
    // Called at the start of #pragma omp parallel on a distributed worker.
    void start_parallel(
        const omp_func_type func
    ,   const omp_data_type data
    ) {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::start_parallel;
        info.func = func;
        info.data = data;
        
        this->set_cmd_and_suspend(info);
    }
    
    // Called at the end of #pragma omp parallel on a distributed worker.
    void end_parallel()
    {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::end_parallel;
        
        this->set_cmd_and_suspend(info);
    }
    
    // Called at the end of #pragma omp parallel on child workers.
    void exit_parallel()
    {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::exit_parallel;
        
        this->set_cmd_and_exit(info);
    }
    
    void exit_program()
    {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::exit_program;
        
        this->set_cmd_and_exit(info);
    }
    
    cmd_info_type wait_for_cmd()
    {
        MEFDN_ASSERT(this->is_cmd_ready_);
        
        const auto cmd = this->cmd_info_;
        this->cmd_info_ = cmd_info_type();
        return cmd;
    }
    
    void reset_cmd()
    {
        this->cmd_info_ = cmd_info_type();
        MEFDN_ASSERT(this->is_cmd_ready_);
        this->is_cmd_ready_ = false;
        
        this->start_user_code_on_this_thread();
        
        // Resume the ULT.
        this->resume();
        
        this->end_user_code_on_this_thread();
    }
    
private:
    void set_cmd_and_suspend(const cmd_info_type& cmd_info)
    {
        MEFDN_ASSERT(cmd_info.code != cmd_code_type::none);
        
        this->cmd_info_ = cmd_info;
        this->is_cmd_ready_ = true;
        
        this->suspend();
        
        MEFDN_ASSERT(!this->is_cmd_ready_);
    }
    void set_cmd_and_exit(const cmd_info_type& cmd_info)
    {
        MEFDN_ASSERT(cmd_info.code != cmd_code_type::none);
        
        this->cmd_info_ = cmd_info;
        this->is_cmd_ready_ = true;
        
        this->exit();
    }
    
    void start_user_code_on_this_thread()
    {
        // Initialize the TLS to resume the application code.
        this->initialize_tls();
        
        this->work_tk_.pin(P::get_dsm_space());
    }
    
    void end_user_code_on_this_thread()
    {
        this->work_tk_.unpin(P::get_dsm_space());
        
        // Finalize the TLS because this function returns to the system code.
        this->finalize_tls();
    }
    
private:
    int thread_num_ = 0;
    int num_threads_ = 1;
    
    task_ref_type work_tk_;
    
    cmd_info_type   cmd_info_ = cmd_info_type();
    bool            is_cmd_ready_ = false;
};

} // namespace meomp
} // namespace menps

