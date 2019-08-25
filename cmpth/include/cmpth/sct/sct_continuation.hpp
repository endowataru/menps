
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class sct_continuation
{
    using task_desc_type = typename P::task_desc_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
    using context_type = typename P::context_type;
    
public:
    sct_continuation() noexcept = default;
    
    explicit sct_continuation(unique_task_ptr_type tk) noexcept
        : tk_{fdn::move(tk)}
    { }
    
    sct_continuation(const sct_continuation&) = delete;
    sct_continuation& operator = (const sct_continuation&) = delete;
    
    sct_continuation(sct_continuation&&) noexcept = default;
    sct_continuation& operator = (sct_continuation&&) noexcept = default;
    
    context_type get_context() const noexcept {
        return this->tk_->ctx;
    }
    
    task_desc_type* release() noexcept {
        return this->tk_.release();
    }
    
    explicit operator bool() const noexcept {
        return !!tk_;
    }
    
private:
    unique_task_ptr_type tk_;
};

} // namespace cmpth

