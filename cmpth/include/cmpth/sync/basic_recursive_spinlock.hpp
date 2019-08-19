
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_recursive_spinlock
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using base_spinlock_type = typename P::base_spinlock_type;
    using lock_guard_type = fdn::lock_guard<base_spinlock_type>;
    using thread_id_type = typename base_ult_itf_type::thread::id;
    
public:
    basic_recursive_spinlock() noexcept = default;
    
    basic_recursive_spinlock(const basic_recursive_spinlock&) = delete;
    basic_recursive_spinlock& operator = (const basic_recursive_spinlock&) = delete;
    
    CMPTH_NODISCARD
    bool try_lock() noexcept
    {
        const auto this_tid = base_ult_itf_type::this_thread::get_id();
        
        lock_guard_type lk{this->lock_};
        if (this->tid_ == this_tid) {
            // This thread already owns this lock.
            ++this->count_;
            return true;
        }
        else if (this->count_ == 0) {
            // No thread is locking this lock.
            this->tid_ = base_ult_itf_type::this_thread::get_id();
            this->count_ = 1;
            return true;
        }
        else {
            return false;
        }
    }

    void lock() noexcept {
        while (!this->try_lock()) {
            base_ult_itf_type::this_thread::yield();
        }
    }
    
    void unlock() noexcept {
        lock_guard_type lk{this->lock_};
        CMPTH_P_ASSERT(P, this->tid_ == base_ult_itf_type::this_thread::get_id());
        CMPTH_P_ASSERT(P, this->count_ > 0);
        --this->count_;
    }
    
private:
    base_spinlock_type  lock_;
    thread_id_type      tid_;
    fdn::size_t         count_ = 0;
};

} // namespace cmpth

