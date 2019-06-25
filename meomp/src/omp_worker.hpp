
#pragma once

#include <menps/meomp/common.hpp>

//#define MEOMP_CALL_PIN_ON_WORKER_THREAD

namespace menps {
namespace meomp {

template <typename P>
class omp_worker
#ifdef MEOMP_USE_CMPTH
    : public P::single_worker_type
#else
    : public P::single_ult_worker_type
    , public P::thread_specific_worker_type
    , public P::context_policy_type
#endif
{
    MEFDN_DEFINE_DERIVED(P)
    
    #ifdef MEOMP_USE_CMPTH
    using task_ref_type = typename P::task_ref_type;
    using call_stack_type = typename P::call_stack_type;
    #else
    using global_ult_ref_type = typename P::ult_ref_type; // TODO
    #endif
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    #ifndef MEOMP_USE_CMPTH
    using context_type = typename P::context_type;
    #endif
    
public:
    #ifdef MEOMP_USE_CMPTH
    // for compatibility
    static derived_type& get_current_worker() noexcept {
        return derived_type::get_cur_worker();
    }
    
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
    #endif
    
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
    void start_worker(
    #ifndef MEOMP_USE_CMPTH
        global_ult_ref_type root_gu
    ,   global_ult_ref_type work_gu
    ,   
    #endif
        Func&&              func
    ) {
        auto& self = this->derived();
        
        #ifndef MEOMP_USE_CMPTH
        this->root_gu_ = mefdn::move(root_gu);
        this->work_gu_ = mefdn::move(work_gu);
        #endif
        
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
        #ifndef MEOMP_USE_CMPTH
        // Reset these members.
        this->root_gu_ = global_ult_ref_type();
        this->work_gu_ = global_ult_ref_type();
        #endif
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
        #ifdef MEOMP_USE_CMPTH
        // Initialize the TLS to resume the application code.
        this->initialize_tls();
        
        this->work_tk_.pin(P::get_dsm_space());
        
        #else
        // Initialize the TLS to resume the application code.
        this->initialize_on_this_thread();
        
        #ifndef MEOMP_CALL_PIN_ON_WORKER_THREAD
        this->get_work_ult().pin();
        #endif
        #endif
    }
    
    void end_user_code_on_this_thread()
    {
        #ifdef MEOMP_USE_CMPTH
        this->work_tk_.unpin(P::get_dsm_space());
        
        // Finalize the TLS because this function returns to the system code.
        this->finalize_tls();
        #else
        #ifndef MEOMP_CALL_PIN_ON_WORKER_THREAD
        this->get_work_ult().unpin();
        #endif
        
        // Finalize the TLS because this function returns to the system code.
        this->finalize_on_this_thread();
        #endif
    }
    
private:
    #ifndef MEOMP_USE_CMPTH
    friend typename P::single_ult_worker_type;
    friend typename P::ult_switcher_type;
    
    // Used in mectx::single_ult_worker.
    global_ult_ref_type& get_root_ult() { return this->root_gu_; }
    global_ult_ref_type& get_work_ult() { return this->work_gu_; }
    
    // Used in mectx::ult_switcher.
    void on_before_switch(global_ult_ref_type& /*from_th*/, global_ult_ref_type& to_th MEFDN_MAYBE_UNUSED) {
        #ifdef MEOMP_CALL_PIN_ON_WORKER_THREAD
        to_th.pin();
        #endif
    }
    void on_after_switch(global_ult_ref_type& from_th MEFDN_MAYBE_UNUSED, global_ult_ref_type& /*to_th*/) {
        #ifdef MEOMP_CALL_PIN_ON_WORKER_THREAD
        from_th.unpin();
        #endif
    }
    
    // Used in mectx::ult_switcher.
    context_type get_context(global_ult_ref_type& th) {
        return th.get_context();
    }
    void set_context(global_ult_ref_type& th, context_type ctx) {
        th.set_context(ctx);
    }
    
    // Used in mectx::ult_switcher.
    std::string show_ult_ref(global_ult_ref_type&) {
        return ""; // TODO
    }
    void* get_stack_ptr(global_ult_ref_type& th) {
        return th.get_stack_ptr();
    }
    mefdn::size_t get_stack_size(global_ult_ref_type& th) {
        return th.get_stack_size();
    }
    #endif
    
    int thread_num_ = 0;
    int num_threads_ = 1;
    
    #ifdef MEOMP_USE_CMPTH
    task_ref_type work_tk_;
    
    #else
    global_ult_ref_type root_gu_;
    global_ult_ref_type work_gu_;
    #endif
    
    cmd_info_type   cmd_info_ = cmd_info_type();
    bool            is_cmd_ready_ = false;
};

} // namespace meomp
} // namespace menps

