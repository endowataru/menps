
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_thread_funcs
{
    using worker_type = typename P::worker_type;
    
    using context_policy_type = typename P::context_policy_type;
    using context_type = typename P::context_type;
    using transfer_type = typename P::transfer_type;
    
    using continuation_type = typename P::continuation_type;
    using call_stack_type = typename P::call_stack_type;
    using task_ref_type = typename P::task_ref_type;
    using task_desc_type = typename P::task_desc_type;
    
    using task_pool_type = typename P::task_pool_type;
    
    template <typename Func>
    struct on_fork_child_first {
        [[noreturn]] // TODO
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   parent_cont
        ,   Func* const         func
        ) {
            wk.local_push_top( fdn::move(parent_cont) );
            
            auto ret = (*func)();
            
            // Explicitly destruct the function object.
            func->~Func();
            
            basic_thread_funcs::exit(ret.wk, ret.pool);
        }
    };
    
public:
    template <typename Func>
    static task_ref_type fork_child_first(
        worker_type&    wk
    ,   call_stack_type child_stk
    ,   Func&&          func
    ) {
        using func_type = fdn::decay_t<Func>;
        
        auto& f =
            child_stk.template construct<func_type>(
                fdn::forward<Func>(func)
            );
        
        const auto child_tk = child_stk.get_task_ref();
        
        wk.template suspend_to_new<
            basic_thread_funcs::on_fork_child_first<func_type>
        >
        (fdn::move(child_stk), &f);
        
        return child_tk;
    }
    
private:
    template <typename Func>
    struct on_fork_parent_first {
        [[noreturn]]
        void operator() (
            transfer_type   /*tr*/
        ,   Func* const     func
        ) {
            auto ret = (*func)();
            
            // Explicitly destruct the function object.
            func->~Func();
            
            basic_thread_funcs::exit(ret.wk, ret.pool);
        }
    };
    
public:
    template <typename Func>
    static task_ref_type fork_parent_first(
        worker_type&    wk
    ,   call_stack_type child_stk
    ,   Func&&          func
    ) {
        using func_type = fdn::decay_t<Func>;
        
        auto& f =
            child_stk.template construct<func_type>(
                fdn::forward<Func>(func)
            );
        
        auto ctx =
            context_policy_type::template make_context<
                basic_thread_funcs::on_fork_parent_first<func_type>
            ,   worker_type
            > (
                child_stk.get_stack_ptr()
            ,   child_stk.get_stack_size()
            ,   &f
            );
        
        const auto child_tk = child_stk.get_task_ref();
        
        auto child_cont =
            fdn::move(child_stk).make_continuation(ctx);
        
        wk.local_push_top( fdn::move(child_cont) );
        
        return child_tk;
    }
    
private:
    struct on_exit {
        worker_type& operator() (
            worker_type&        wk
        ,   task_ref_type       prev_tk
        ,   task_pool_type*     pool
        ) const {
            {
                auto lk = prev_tk.get_lock(fdn::adopt_lock);
                
                if (prev_tk.is_detached(lk)) {
                    // Deallocate the exiting thread.
                    pool->deallocate(wk, prev_tk);
                }
                else {
                    // Notify the joiner thread.
                    prev_tk.set_finished_release(lk);
                }
                
                // lk's destructor unlocks prev_stk.
            }
            
            return wk;
        }
    };
    
public:
    [[noreturn]]
    static void exit(
        worker_type&    wk
    ,   task_pool_type& pool
    ) {
        auto cur_tk = wk.get_cur_task_ref();
        
        auto lk = cur_tk.get_lock();
        
        if (auto cont = cur_tk.try_get_joiner(lk)) {
            // Exit to the joiner thread.
            wk.template exit_to_cont<
                basic_thread_funcs::on_exit
            >(fdn::move(cont), &pool);
        }
        else {
            // Exit to the scheduler.
            wk.template exit_to_sched<
                basic_thread_funcs::on_exit
            >(&pool);
        }
        
        // Be careful that the destructors are not called in this function.
        CMPTH_UNREACHABLE();
    }
    
private:
    struct on_join {
        worker_type& operator() (
            worker_type&            wk
        ,   continuation_type       joiner_cont
        ,   task_desc_type* const   joined_desc
        ) {
            {
                task_ref_type joined_tk{joined_desc};
                
                auto lk = joined_tk.get_lock(fdn::adopt_lock);
                
                // Set the blocking thread continued by the child thread.
                joined_tk.set_joiner(lk, fdn::move(joiner_cont));
                
                // lk's destructor unlocks joined_desc.
            }
            
            return wk;
        }
    };
    
public:
    static worker_type& join(
        worker_type&        wk
    ,   task_pool_type&     pool
    ,   const task_ref_type joined_tk
    ) {
        // wk.current joins "this".
        
        // TODO: This optimization was buggy
        // 1st check without lock
        /*if (joined_tk.is_finished_acquire()) {
            pool.deallocate(wk, joined_tk);
            return wk;
        }*/
        
        {
            auto lk = joined_tk.get_lock();
            
            // 2nd check with lock
            if (joined_tk.is_finished(lk)) {
                lk.unlock();
                
                pool.deallocate(wk, joined_tk);
                return wk;
            }
            
            lk.release();
        }
        
        auto joined_desc = joined_tk.get_task_desc();
        
        auto& wk_2 =
            wk.template suspend_to_sched<
                basic_thread_funcs::on_join
            >(joined_desc);
        
        return wk_2;
    }
};

} // namespace cmpth

