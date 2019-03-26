
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class smth_call_stack
{
    using continuation_type = typename P::continuation_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    using task_desc_type = typename P::task_desc_type;
    using task_ref_type = typename P::task_ref_type;
    
    using context_type = typename P::context_type;
    
public:
    smth_call_stack() noexcept = default;
    
    explicit smth_call_stack(unique_task_ptr_type tk)
        : tk_{fdn::move(tk)}
        , ptr_{this->tk_->stk_bottom}
    {
        const auto diff =
            static_cast<fdn::byte*>(this->tk_->stk_bottom) -
            static_cast<fdn::byte*>(this->tk_->stk_top);
        
        CMPTH_P_ASSERT(P, diff > 0);
        
        this->size_ = static_cast<fdn::size_t>(diff);
    }
    
    // move-only
    
    template <typename T, typename... Args>
    T& construct(Args&&... args)
    {
        CMPTH_P_ASSERT(P, this->tk_);
        
        const auto p_void =
            fdn::align_call_stack(
                alignof(T), sizeof(T), this->ptr_, this->size_
            );
        
        return * new (p_void) T(fdn::forward<Args>(args)...);
    }
    
    void* get_stack_ptr() const noexcept {
        return this->ptr_;
    }
    fdn::size_t get_stack_size() const noexcept {
        return this->size_;
    }
    
    task_ref_type get_task_ref() const noexcept {
        return { tk_.get() };
    }
    
    continuation_type make_continuation(context_type ctx) && noexcept
    {
        this->tk_->ctx = ctx;
        
        return continuation_type{ fdn::move(this->tk_) };
    }
    
    bool is_detached() noexcept {
        return this->tk_->detached;
    }
    
    task_desc_type* release() noexcept {
        return tk_.release();
    }
    
private:
    unique_task_ptr_type    tk_;
    void*                   ptr_ = nullptr;
    fdn::size_t             size_ = 0;
};

} // namespace cmpth

