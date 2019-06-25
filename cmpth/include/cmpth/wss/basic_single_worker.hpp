
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_single_worker
    : public P::worker_task_type
    , public P::worker_tls_type
{
    CMPTH_DEFINE_DERIVED(P)
    
    using call_stack_type = typename P::call_stack_type;
    using continuation_type = typename P::continuation_type;
    using task_ref_type = typename P::task_ref_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
public:
    template <typename Func, typename... Args>
    void execute(Args* const ... args)
    {
        auto& self = this->derived();
        using func_type = fdn::decay_t<Func>;
        
        self.template suspend_to_new<
            basic_single_worker::on_execute<Func, Args...>
        >
        (fdn::move(this->work_stk_), args...);
    }
    
private:
    template <typename Func, typename... Args>
    struct on_execute
    {
        [[noreturn]]
        derived_type& operator() (
            derived_type&       self
        ,   continuation_type   cont
        ,   Args* const ...     args
        ) const
        {
            self.root_cont_ = fdn::move(cont);
            
            Func{}(self, args...);
            
            CMPTH_UNREACHABLE();
        }
    };
    
public:
    void suspend()
    {
        auto& self = this->derived();
        self.template suspend_to_cont<on_suspend>(
            fdn::move(this->root_cont_)
        );
    }
    
private:
    struct on_suspend
    {
        derived_type& operator() (
            derived_type&       self
        ,   continuation_type   cont
        ) const
        {
            self.work_cont_ = fdn::move(cont);
            
            return self;
        }
    };
    
public:
    void resume()
    {
        auto& self = this->derived();
        
        self.template suspend_to_cont<on_resume>(
            fdn::move(this->work_cont_)
        );
    }
    
private:
    struct on_resume
    {
        derived_type& operator() (
            derived_type&       self
        ,   continuation_type   cont
        ) const
        {
            self.root_cont_ = fdn::move(cont);
            
            return self;
        }
    };
    
public:
    void exit()
    {
        auto& self = this->derived();
        
        self.template exit_to_cont<on_exit>(
            fdn::move(this->root_cont_)
        );
    }
    
private:
    struct on_exit
    {
        derived_type& operator() (
            derived_type&   self
        ,   task_ref_type   prev_tk
        ) const
        {
            self.work_stk_ =
                call_stack_type{
                    unique_task_ptr_type{prev_tk.get_task_desc()}
                };
            
            return self;
        }
    };
    
public:
    void set_work_stack(call_stack_type work_stk) {
        this->work_stk_ = fdn::move(work_stk);
    }
    call_stack_type unset_work_stack() {
        return fdn::move(this->work_stk_);
    }
    
    task_ref_type get_work_task_ref() {
        return this->work_cont_.get_task_desc();
    }
    
private:
    continuation_type   root_cont_;
    continuation_type   work_cont_;
    call_stack_type     work_stk_;
};

} // namespace cmpth

