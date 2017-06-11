
#pragma once

#include "ult_desc.hpp"
#include <mgult/ult_id.hpp>

#include <mgbase/threading/unique_lock.hpp>
#include <mgbase/type_traits/underlying_type.hpp>
#include <mgbase/utility/move.hpp>

#include <mgbase/external/fmt.hpp>
#include <string>

namespace mgult {
namespace sm {

class ult_ptr_ref
{
    typedef ult_desc::lock_type     lock_type;
    typedef context_t               context_type;
    
public:
    typedef mgbase::unique_lock<lock_type>  unique_lock_type;
    
    ult_ptr_ref() MGBASE_NOEXCEPT {
        set_invalid();
    }
    
    explicit ult_ptr_ref(const ult_id& id) {
        set_id(id);
    }
    
    ~ult_ptr_ref() /*noexcept*/ = default;
    
    ult_ptr_ref(const ult_ptr_ref&) = delete;
    ult_ptr_ref& operator = (const ult_ptr_ref&) = delete;
    
    ult_ptr_ref(ult_ptr_ref&& other) MGBASE_NOEXCEPT {
        set_invalid();
        *this = mgbase::move(other);
    }
    ult_ptr_ref& operator = (ult_ptr_ref&& other) MGBASE_NOEXCEPT {
        desc_ = other.desc_;
        other.set_invalid();
        return *this;
    }
    
    void* get_stack_ptr() const MGBASE_NOEXCEPT {
        return desc_->stack_ptr;
    }
    mgbase::size_t get_stack_size() {
        return desc_->stack_size;
    }
    
    template <typename... Args>
    unique_lock_type get_lock(Args&&... args)
    {
        unique_lock_type lk(desc_->lock, mgbase::forward<Args>(args)...);
        return lk;
    }
    
    void set_blocked(unique_lock_type& lk) {
        check_locked(lk);
        MGBASE_ASSERT(desc_->state == ult_state::ready);
        
        desc_->state = ult_state::blocked;
    }
    void set_ready(unique_lock_type& lk) {
        check_locked(lk);
        MGBASE_ASSERT(desc_->state == ult_state::blocked);
        
        desc_->state = ult_state::ready;
    }
    void set_finished(unique_lock_type& lk) {
        check_locked(lk);
        MGBASE_ASSERT(desc_->state == ult_state::ready);
        
        desc_->state = ult_state::finished;
    }
    
    bool is_finished(unique_lock_type& lk) const {
        check_locked(lk);
        return desc_->state == ult_state::finished;
    }
    bool is_latest_stamp(unique_lock_type& lk) const {
        check_locked(lk);
        return true; // this function is for distributed-memory version
    }
    
    bool is_detached(unique_lock_type& lk) const {
        check_locked(lk);
        return desc_->detached;
    }
    void set_detached(unique_lock_type& lk) {
        check_locked(lk);
        desc_->detached = false;
    }
    
    bool has_joiner(unique_lock_type& lk) const {
        check_locked(lk);
        return ! is_invalid_ult_id(this->get_joiner(lk).get_id());
    }
    void set_joiner(unique_lock_type& lk, const ult_ptr_ref& joiner) {
        check_locked(lk);
        MGBASE_ASSERT(joiner.desc_ != MGBASE_NULLPTR);
        desc_->joiner = joiner.desc_;
    }
    ult_ptr_ref get_joiner(unique_lock_type& lk) const {
        check_locked(lk);
        return ult_ptr_ref(make_desc_id(*desc_->joiner));
    }
    
    bool is_valid() const MGBASE_NOEXCEPT {
        return desc_ != ult_ptr_ref{}.desc_;
    }
    
    ult_id get_id() const MGBASE_NOEXCEPT {
        return { desc_ };
    }
    
    std::string to_string()
    {
        MGBASE_ASSERT(is_valid());
        
        fmt::MemoryWriter w;
        w.write(
            "id:{:x}\t"
            "state:{}\t"
            "joiner:{:x}\t"
            "detached:{}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_)
        ,   static_cast<ult_state_underlying_t>(desc_->state)
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_->joiner)
        ,   desc_->detached
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_->stack_ptr)
        ,   desc_->stack_size
        );
        return w.str();
    }
    
    // Avoid using this if possible
    ult_desc& get_desc() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(is_valid());
        return *desc_;
    }
    
private:
    void check_locked(unique_lock_type& lk) const {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(&desc_->lock == lk.mutex());
        MGBASE_ASSERT(lk.owns_lock());
    }
    
    void set_invalid() MGBASE_NOEXCEPT {
        set_id(make_invalid_ult_id());
    }
    void set_id(const ult_id& id) MGBASE_NOEXCEPT {
        desc_ = static_cast<ult_desc*>(id.ptr);
    }
    
    static ult_id make_desc_id(ult_desc& desc) {
        return { &desc };
    }
    
    ult_desc* desc_;
};

} // namespace sm
} // namespace mgult

