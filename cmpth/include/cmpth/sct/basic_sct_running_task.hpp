
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_sct_running_task
{
    using task_desc_type = typename P::task_desc_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    using task_ref_type = typename P::task_ref_type;
    
    using continuation_type = typename P::continuation_type;
    
    using context_type = typename P::context_type;
    
public:
    basic_sct_running_task() noexcept
        : root_desc_()
        // Note: GCC 4.8 does NOT zero-fill empty {}
        , tk_{&this->root_desc_}
    { }
    
    ~basic_sct_running_task() {
        // Check that the root task is deleted.
        CMPTH_P_ASSERT(P, this->tk_.get() == &this->root_desc_);
        // No need to track the ownership of the root task's descriptor.
        this->tk_.release();
    }
    
    // uncopyable
    basic_sct_running_task(const basic_sct_running_task&) = delete;
    basic_sct_running_task& operator = (const basic_sct_running_task&) = delete;
    
    continuation_type suspend_to(
        const context_type      cur_ctx
    ,   task_desc_type* const   next_tk
    ) {
        CMPTH_P_ASSERT(P, this->tk_);
        CMPTH_P_ASSERT(P, next_tk != nullptr);
        unique_task_ptr_type tk{next_tk};
        
        using fdn::swap;
        swap(this->tk_, tk);
        
        // Set the current context to the descriptor.
        tk->ctx = cur_ctx;
        
        return continuation_type{fdn::move(tk)};
    }

    continuation_type revive_suspended(continuation_type prev_cont)
    {
        CMPTH_P_ASSERT(P, this->tk_);
        CMPTH_P_ASSERT(P, prev_cont);
        unique_task_ptr_type tk{prev_cont.release()};
        
        using fdn::swap;
        swap(this->tk_, tk);
        
        return continuation_type{fdn::move(tk)};
    }
    
    task_ref_type exit_to(task_desc_type* const next_tk)
    {
        CMPTH_P_ASSERT(P, next_tk != nullptr);
        unique_task_ptr_type tk{next_tk};
        
        CMPTH_P_ASSERT(P, this->tk_);
        
        using fdn::swap;
        swap(this->tk_, tk);
        
        // The context is reset for debugging.
        // Note: GCC 4.8 needs to use () to value-initialize.
        tk->ctx = context_type();
        
        return { tk.release() };
    }
    
    task_ref_type get_task_ref() const noexcept
    {
        return { tk_.get() };
    }
    
    void mark_as_root() noexcept {
        CMPTH_P_ASSERT(P, this->tk_);
        CMPTH_P_ASSERT(P, !this->tk_->is_root);
        this->tk_->is_root = true;
    }
    void unmark_as_root() noexcept {
        CMPTH_P_ASSERT(P, this->tk_);
        CMPTH_P_ASSERT(P, this->tk_->is_root);
        this->tk_->is_root = false;
    }
    
private:
    task_desc_type          root_desc_;
    unique_task_ptr_type    tk_;
};

} // namespace cmpth

