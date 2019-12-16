
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_sct_continuation
{
    using task_desc_type = typename P::task_desc_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
    using context_type = typename P::context_type;
    
public:
    basic_sct_continuation() noexcept = default;
    
    explicit basic_sct_continuation(unique_task_ptr_type tk) noexcept
        : tk_{fdn::move(tk)}
    { }
    
    basic_sct_continuation(const basic_sct_continuation&) = delete;
    basic_sct_continuation& operator = (const basic_sct_continuation&) = delete;
    
    basic_sct_continuation(basic_sct_continuation&&) noexcept = default;
    basic_sct_continuation& operator = (basic_sct_continuation&&) noexcept = default;
    
    context_type get_context() const noexcept {
        CMPTH_P_ASSERT(P, *this);
        return this->tk_->ctx;
    }
    
    task_desc_type* release() noexcept {
        return this->tk_.release();
    }
    
    explicit operator bool() const noexcept {
        return !!tk_;
    }
    
    bool is_root() const noexcept {
        CMPTH_P_ASSERT(P, *this);
        return this->tk_->is_root;
    }
    
private:
    unique_task_ptr_type tk_;
};

} // namespace cmpth

