
#pragma once

#include <menps/meomp/common.hpp>

namespace menps {
namespace meomp {

template <typename P>
struct task_desc
{
private:
    using context_type = typename P::context_type;
    
public:
    context_type    ctx;
    void*           stk_top;
    void*           stk_bottom;
};

template <typename P>
class task_ref
{
    using task_desc_type = typename P::task_desc_type;
    
public:
    task_ref() noexcept = default;
    
    /*implicit*/ task_ref(task_desc_type* const desc) noexcept
        : desc_{desc}
    { }
    
    task_ref(const task_ref&) noexcept = default;
    task_ref& operator = (const task_ref&) noexcept = default;
    
    task_desc_type* get_task_desc() const noexcept {
        CMPTH_P_ASSERT(P, *this);
        return this->desc_;
    }
    explicit operator bool() const noexcept {
        return this->desc_;
    }
    
    template <typename Space>
    void pin(Space& sp) {
        sp.pin(this->desc_->stk_top, this->get_stack_size());
    }
    
    template <typename Space>
    void unpin(Space& sp) {
        sp.unpin(this->desc_->stk_top, this->get_stack_size());
    }
    
private:
    mefdn::size_t get_stack_size() const noexcept {
        return
            static_cast<mefdn::byte*>(this->desc_->stk_bottom) -
            static_cast<mefdn::byte*>(this->desc_->stk_top);
    }
    
    task_desc_type* desc_{nullptr};
};

} // namespace meomp
} // namespace menps

