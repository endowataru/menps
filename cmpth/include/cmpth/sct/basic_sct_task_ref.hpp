
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_sct_task_ref
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using task_desc_type = typename P::task_desc_type;
    using task_mutex_type = typename P::task_mutex_type;
    using task_lock_type =
        typename base_ult_itf_type::template unique_lock<task_mutex_type>;
    
    using continuation_type = typename P::continuation_type;
    
public:
    basic_sct_task_ref() noexcept = default;
    
    /*implicit*/ basic_sct_task_ref(task_desc_type* const desc) noexcept
        : desc_{desc}
    { }
    
    basic_sct_task_ref(const basic_sct_task_ref&) noexcept = default;
    basic_sct_task_ref& operator = (const basic_sct_task_ref&) noexcept = default;
    
    explicit operator bool() const noexcept {
        return this->desc_;
    }
    
    task_desc_type* get_task_desc() const noexcept {
        CMPTH_P_ASSERT(P, *this);
        return this->desc_;
    }
    
    task_lock_type get_lock() const {
        return task_lock_type{ this->desc_->mtx };
    }
    template <typename Tag>
    task_lock_type get_lock(Tag tag) const {
        return task_lock_type{ this->desc_->mtx, tag };
    }
    
    void set_finished_release(task_lock_type& lk) noexcept {
        this->check_locked(lk);
        CMPTH_P_ASSERT(P, !this->desc_->finished.load());
        //lk.unlock();
        this->desc_->finished.store(true, fdn::memory_order_release);
        this->desc_ = nullptr;
    }
    bool is_finished_acquire() const noexcept {
        return this->desc_->finished.load(fdn::memory_order_acquire);
    }
    bool is_finished(task_lock_type& lk) const noexcept {
        this->check_locked(lk);
        return this->desc_->finished.load(fdn::memory_order_relaxed);
    }
    
    void set_detached(task_lock_type& lk) const noexcept {
        this->check_locked(lk);
        this->desc_->detached = true;
    }
    bool is_detached(task_lock_type& lk) const noexcept {
        this->check_locked(lk);
        return this->desc_->detached;
    }
    
    void set_joiner(task_lock_type& lk, continuation_type joiner_cont) const noexcept {
        this->check_locked(lk);
        this->desc_->joiner = fdn::move(joiner_cont);
    }
    continuation_type try_get_joiner(task_lock_type& lk) const noexcept {
        this->check_locked(lk);
        return fdn::move(this->desc_->joiner);
    }
    
private:
    void check_locked(task_lock_type& lk) const noexcept {
        CMPTH_P_ASSERT(P, lk.mutex() == &this->desc_->mtx);
        CMPTH_P_ASSERT(P, lk.owns_lock());
    }
    
    task_desc_type* desc_{nullptr};
};

} // namespace cmpth

