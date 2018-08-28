
#pragma once

#include <menps/meomp/common.hpp>

//#define MEOMP_CALL_PIN_ON_WORKER_THREAD

namespace menps {
namespace meomp {

template <typename P>
class omp_worker
    : public P::single_ult_worker_type
    , public P::thread_specific_worker_type
    , public P::context_policy_type
{
    MEFDN_DEFINE_DERIVED(P)
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    using klt_itf_type = typename P::worker_ult_itf_type; // TODO: use consistent name
    using klt_thread_type = typename klt_itf_type::thread;
    using klt_mutex_type = typename klt_itf_type::mutex;
    using klt_cv_type = typename klt_itf_type::condition_variable;
    using klt_unique_lock_type = typename klt_itf_type::unique_mutex_lock; // TODO : rename
    #endif
    
    using global_ult_ref_type = typename P::ult_ref_type; // TODO
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    using context_type = typename P::context_type;
    
public:
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
        global_ult_ref_type root_gu
    ,   global_ult_ref_type work_gu
    ,   Func&&              func
    ) {
        auto& self = this->derived();
        
        this->root_gu_ = mefdn::move(root_gu);
        this->work_gu_ = mefdn::move(work_gu);
        
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        // Pin the call stack.
        self.start_delegated_code();
        
        // Start the kernel thread.
        this->root_ku_ =
            klt_thread_type(
                delegated_main<mefdn::decay_t<Func>>{
                    self
                ,   // Copy the func so that it can survive
                    // after start_worker() function exits.
                    mefdn::forward<Func>(func)
                }
            );
        #else
        // Execute this function object immediately.
        delegated_main<mefdn::decay_t<Func>>{
            self
        ,   // Copy the func so that it can survive
            // after start_worker() function exits.
            mefdn::forward<Func>(func)
        }();
        #endif
    }
    void end_worker()
    {
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        this->root_ku_.join();
        
        // This unpin is unnecessary
        // because it's already done in wait_cmd().
        /*// Unpin the call stack.
        this->end_delegated_code();*/
        #endif
        
        // Reset these members.
        this->root_gu_ = global_ult_ref_type();
        this->work_gu_ = global_ult_ref_type();
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
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    bool try_upgrade(void* const ptr)
    {
        cmd_info_type info = cmd_info_type();
        info.code = cmd_code_type::try_upgrade;
        info.data = ptr;
        
        this->set_cmd_and_suspend(info);
        
        // TODO: Always returns "true" now.
        return true;
    }
    #endif
    
    cmd_info_type wait_for_cmd() {
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        cmd_info_type cmd_info = cmd_info_type();
        {
            klt_unique_lock_type lk(this->cmd_mtx_);
            while (! this->is_cmd_ready_) {
                this->cmd_cv_.wait(lk);
            }
            cmd_info = this->cmd_info_;
        }
        
        // Unpin the call stack.
        this->end_delegated_code();
        
        return cmd_info;
        
        #else
        MEFDN_ASSERT(this->is_cmd_ready_);
        
        const auto cmd = this->cmd_info_;
        this->cmd_info_ = cmd_info_type();
        return cmd;
        #endif
    }
    
    void reset_cmd() {
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        // Pin the call stack again.
        this->start_delegated_code();
        
        {
            klt_unique_lock_type lk(this->cmd_mtx_);
            this->cmd_info_ = cmd_info_type{};
            MEFDN_ASSERT(this->is_cmd_ready_);
            this->is_cmd_ready_ = false;
            
            // Resume the worker thread.
            this->cmd_cv_.notify_one();
        }
        
        #else
        this->cmd_info_ = cmd_info_type();
        MEFDN_ASSERT(this->is_cmd_ready_);
        this->is_cmd_ready_ = false;
        
        this->start_user_code_on_this_thread();
            
        // Resume the ULT.
        this->resume();
        
        this->end_user_code_on_this_thread();
        #endif
    }
    
private:
    void set_cmd_and_suspend(const cmd_info_type& cmd_info)
    {
        MEFDN_ASSERT(cmd_info.code != cmd_code_type::none);
        
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        {
            klt_unique_lock_type lk(this->cmd_mtx_);
            MEFDN_ASSERT(!this->is_cmd_ready_);
            this->cmd_info_ = cmd_info;
            this->is_cmd_ready_ = true;
            this->cmd_cv_.notify_one();
            
            while (this->is_cmd_ready_) {
                this->cmd_cv_.wait(lk);
            }
        }
        #else
        this->cmd_info_ = cmd_info;
        this->is_cmd_ready_ = true;
        
        this->suspend();
        
        MEFDN_ASSERT(!this->is_cmd_ready_);
        #endif
    }
    void set_cmd_and_exit(const cmd_info_type& cmd_info)
    {
        MEFDN_ASSERT(cmd_info.code != cmd_code_type::none);
        
        #ifdef MEOMP_SEPARATE_WORKER_THREAD
        {
            klt_unique_lock_type lk(this->cmd_mtx_);
            MEFDN_ASSERT(!this->is_cmd_ready_);
            this->cmd_info_ = cmd_info;
            this->is_cmd_ready_ = true;
            this->cmd_cv_.notify_one();
        }
        #else
        this->cmd_info_ = cmd_info;
        this->is_cmd_ready_ = true;
        #endif
        
        this->exit();
    }
    
    void start_user_code_on_this_thread()
    {
        // Initialize the TLS to resume the application code.
        this->initialize_on_this_thread();
        
        #ifndef MEOMP_CALL_PIN_ON_WORKER_THREAD
        #ifndef MEOMP_SEPARATE_WORKER_THREAD
        this->get_work_ult().pin();
        #endif
        #endif
    }
    
    void end_user_code_on_this_thread()
    {
        #ifndef MEOMP_CALL_PIN_ON_WORKER_THREAD
        #ifndef MEOMP_SEPARATE_WORKER_THREAD
        this->get_work_ult().unpin();
        #endif
        #endif
        
        // Finalize the TLS because this function returns to the system code.
        this->finalize_on_this_thread();
    }
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    void start_delegated_code()
    {
        this->get_work_ult().pin();
    }
    void end_delegated_code()
    {
        this->get_work_ult().unpin();
    }
    #endif
    
private:
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
    
    int thread_num_ = 0;
    int num_threads_ = 1;
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    klt_thread_type root_ku_;
    #endif
    global_ult_ref_type root_gu_;
    global_ult_ref_type work_gu_;
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    klt_mutex_type  cmd_mtx_;
    klt_cv_type     cmd_cv_;
    #endif
    cmd_info_type   cmd_info_ = cmd_info_type();
    bool            is_cmd_ready_ = false;
};

} // namespace meomp
} // namespace menps

