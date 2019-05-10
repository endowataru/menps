
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class smth_running_task
{
    using task_desc_type = typename P::task_desc_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    using task_ref_type = typename P::task_ref_type;
    
    using call_stack_type = typename P::call_stack_type;
    using continuation_type = typename P::continuation_type;
    
    using context_type = typename P::context_type;
    
public:
    smth_running_task() noexcept
        : root_desc_()
        // Note: GCC 4.8 does NOT zero-fill empty {}
        , tk_{&this->root_desc_}
    { }
    
    ~smth_running_task() {
        // Check that the root task is deleted.
        CMPTH_P_ASSERT(P, this->tk_.get() == &this->root_desc_);
        // No need to track the ownership of the root task's descriptor.
        this->tk_.release();
    }
    
    // uncopyable
    smth_running_task(const smth_running_task&) = delete;
    smth_running_task& operator = (const smth_running_task&) = delete;
    
    continuation_type suspend_to(
        const context_type      cur_ctx
    ,   task_desc_type* const   next_tk
    ) {
        CMPTH_P_ASSERT(P, next_tk != nullptr);
        unique_task_ptr_type tk{next_tk};
        
        CMPTH_P_ASSERT(P, this->tk_);
        
        using fdn::swap;
        swap(this->tk_, tk);
        
        // Set the current context to the descriptor.
        tk->ctx = cur_ctx;
        
        return continuation_type{fdn::move(tk)};
    }
    
    task_ref_type exit_to(task_desc_type* const next_tk)
    {
        CMPTH_P_ASSERT(P, next_tk != nullptr);
        unique_task_ptr_type tk{next_tk};
        
        CMPTH_P_ASSERT(P, this->tk_);
        
        using fdn::swap;
        swap(this->tk_, tk);
        
        // TODO: Removed for cancel_suspend()
        #if 0
        // The context is reset for debugging.
        tk->ctx = context_type{};
        #endif
        
        return { tk.release() };
    }
    
    task_ref_type get_task_ref() const noexcept
    {
        return { tk_.get() };
    }
    
private:
    task_desc_type          root_desc_;
    unique_task_ptr_type    tk_;
};

} // namespace cmpth

