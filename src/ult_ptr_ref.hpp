
#pragma once

#include <mgult/ult_id.hpp>

#include "ult_desc.hpp"

#include <mgbase/threading/unique_lock.hpp>

#include <mgbase/type_traits/underlying_type.hpp>

namespace mgult {

class my_worker;

class ult_ptr_ref
{
    typedef ult_desc::lock_type     lock_type;
    
    typedef fcontext<my_worker, my_worker>  context_type;
    
public:
    ult_ptr_ref() MGBASE_NOEXCEPT {
        set_invalid();
    }
    
    explicit ult_ptr_ref(const ult_id& id) {
        set_id(id);
    }
    
    ~ult_ptr_ref() = default;
    
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
    
    void set_context(const context_type& ctx) const MGBASE_NOEXCEPT {
        get_desc().ctx = ctx;
    }
    context_type get_context() const MGBASE_NOEXCEPT  {
        return get_desc().ctx;
    }
    
    void* get_stack_ptr() const MGBASE_NOEXCEPT {
        return get_desc().stack_ptr;
    }
    mgbase::size_t get_stack_size() {
        return get_desc().stack_size;
    }
    
    mgbase::unique_lock<lock_type> get_lock()
    {
        mgbase::unique_lock<lock_type> lc(desc_->lock);
        return lc;
    }
    template <typename T>
    mgbase::unique_lock<lock_type> get_lock(const T tag)
    {
        mgbase::unique_lock<lock_type> lc(desc_->lock, tag);
        return lc;
    }
    
    void* get_result()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(is_finished());
        
        return desc_->result;
    }
    void set_result(void* const r)
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(!is_finished());
        
        desc_->result = r;
        desc_->state = ult_state::finished;
    }
    
    void set_blocked()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(desc_->state == ult_state::ready);
        
        desc_->state = ult_state::blocked;
    }
    void set_ready()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(desc_->state == ult_state::blocked);
        
        desc_->state = ult_state::ready;
    }
    
    bool is_finished() const MGBASE_NOEXCEPT {
        return desc_->state == ult_state::finished;
    }
    
    bool is_detached() const MGBASE_NOEXCEPT {
        return desc_->detached;
    }
    void set_detached() const MGBASE_NOEXCEPT {
        desc_->detached = false;
    }
    
    bool has_joiner()
    {
        return desc_->joiner != MGBASE_NULLPTR;
    }
    void set_joiner(const ult_ptr_ref& joiner)
    {
        desc_->joiner = joiner.desc_;
    }
    ult_ptr_ref get_joiner() {
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
            "result:{:x}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_)
        ,   static_cast<typename mgbase::underlying_type<ult_state>::type>(desc_->state)
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_->joiner)
        ,   desc_->detached
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_->result)
        ,   reinterpret_cast<mgbase::uintptr_t>(desc_->stack_ptr)
        ,   desc_->stack_size
        );
        return w.str();
    }
    
private:
    void set_invalid() MGBASE_NOEXCEPT {
        set_id(make_invalid_ult_id());
    }
    void set_id(const ult_id& id) MGBASE_NOEXCEPT {
        desc_ = static_cast<ult_desc*>(id.ptr);
    }
    
    static ult_id make_desc_id(ult_desc& desc) {
        return { &desc };
    }
    
    ult_desc& get_desc() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(is_valid());
        return *desc_;
    }
    
    ult_desc* desc_;
};

} // namespace mgult

