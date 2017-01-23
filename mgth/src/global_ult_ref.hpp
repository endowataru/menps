
#pragma once

#include "global_ult_desc.hpp"
#include <mgult/fcontext.hpp>

#include <mgbase/threading/unique_lock.hpp>
#include <string>
#include <mgbase/logger.hpp>

namespace mgth {

class global_ult_ref
{
    typedef context_t   context_type;
    
    typedef mgcom::rma::remote_ptr<global_ult_desc> desc_remote_ptr;
    
    typedef global_ult_ref  lock_type;
    
public:
    global_ult_ref() MGBASE_NOEXCEPT {
        set_invalid();
    }
    
    global_ult_ref(const ult_id& id, const desc_remote_ptr& desc_rptr)
        : id_(id)
        , desc_rptr_(desc_rptr)
    {
        MGBASE_ASSERT(mgcom::valid_process_id(id.di.proc));
    }
    
    ~global_ult_ref() = default;
    
    global_ult_ref(const global_ult_ref&) = delete;
    global_ult_ref& operator = (const global_ult_ref&) = delete;
    
    global_ult_ref(global_ult_ref&& other) MGBASE_NOEXCEPT {
        set_invalid();
        *this = mgbase::move(other);
    }
    global_ult_ref& operator = (global_ult_ref&& other) MGBASE_NOEXCEPT {
        id_ = other.id_;
        desc_rptr_ = other.desc_rptr_;
        
        other.set_invalid();
        return *this;
    }
    
    void set_context(const context_type& ctx) const MGBASE_NOEXCEPT {
        store_desc_member(&global_ult_desc::ctx, ctx);
    }
    context_type get_context() const MGBASE_NOEXCEPT {
        return load_desc_member(&global_ult_desc::ctx);
    }
    
    void* get_stack_ptr() const MGBASE_NOEXCEPT
    {
        return load_desc_member(&global_ult_desc::stack_ptr);
    }
    mgbase::size_t get_stack_size() const MGBASE_NOEXCEPT
    {
        return load_desc_member(&global_ult_desc::stack_size);
    }
    
    mgbase::unique_lock<lock_type> get_lock()
    {
        mgbase::unique_lock<lock_type> lc(*this);
        return lc;
    }
    template <typename T>
    mgbase::unique_lock<lock_type> get_lock(const T tag)
    {
        mgbase::unique_lock<lock_type> lc(*this, tag);
        return lc;
    }
    
    void lock()
    {
        while (!try_lock()) { }
    }
    
    bool try_lock()
    {
        const auto target_proc = get_target_proc();
        const auto lock_rptr = desc_rptr_.member(&global_ult_desc::lock);
        
        MGBASE_LOG_VERBOSE(
            "msg:Trying to lock thread descriptor.\t"
            "{}"
        ,   to_string()
        );
        
        mgcom::rma::atomic_default_t result{};
        
        compare_and_swap(
            target_proc
        ,   lock_rptr
        ,   0
        ,   1
        ,   &result
        );
        
        const bool locked = result == 0;
        
        MGBASE_LOG_VERBOSE(
            "msg:{} thread descriptor.\t"
            "{}"
        ,   (locked ? "Locked" : "Failed to lock")
        ,   to_string()
        );
        
        return locked;
    }
    
    void unlock()
    {
        const auto target_proc = get_target_proc();
        const auto lock_rptr = desc_rptr_.member(&global_ult_desc::lock);
        
        atomic_write(
            target_proc
        ,   lock_rptr
        ,   0
        );
        
        MGBASE_LOG_VERBOSE(
            "msg:Unlocked thread descriptor.\t"
            "{}"
        ,   to_string()
        );
    }
    
    mgcom::process_id_t get_owner_proc()
    {
        const auto proc = load_desc_member(&global_ult_desc::owner);
        return proc;
    }
    void set_owner_proc(const mgcom::process_id_t proc)
    {
        MGBASE_ASSERT(mgcom::valid_process_id(proc));
        store_desc_member(&global_ult_desc::owner, proc);
    }
    
    void set_blocked()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(get_state() == global_ult_state::ready);
        
        store_desc_member(&global_ult_desc::state, global_ult_state::blocked);
    }
    void set_ready()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(get_state() == global_ult_state::blocked);
        
        store_desc_member(&global_ult_desc::state, global_ult_state::ready);
    }
    void set_finished()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(get_state() == global_ult_state::ready);
        
        store_desc_member(&global_ult_desc::state, global_ult_state::finished);
    }
    
    bool is_finished() const MGBASE_NOEXCEPT {
        return get_state() == global_ult_state::finished;
    }
    
    bool is_detached() const MGBASE_NOEXCEPT {
        return load_desc_member(&global_ult_desc::detached) == 1;
    }
    void set_detached() const MGBASE_NOEXCEPT {
        const decltype(load_desc_member(&global_ult_desc::detached)) one = 1;
        store_desc_member(&global_ult_desc::detached, one);
    }
    
    bool has_joiner()
    {
        return !is_invalid_ult_id(get_joiner());
    }
    void set_joiner(const global_ult_ref& joiner)
    {
        store_desc_member(&global_ult_desc::joiner, joiner.get_id());
    }
    ult_id get_joiner() {
        return load_desc_member(&global_ult_desc::joiner);
    }
    
    bool is_valid() const MGBASE_NOEXCEPT {
        return ! mgult::is_invalid_ult_id(id_);
    }
    
    ult_id get_id() const MGBASE_NOEXCEPT {
        return id_;
    }
    
    std::string to_string()
    {
        MGBASE_ASSERT(is_valid());
        MGBASE_ASSERT(mgcom::valid_process_id(id_.di.proc));
        
        fmt::MemoryWriter w;
        w.write(
            "id:{:x}\t"
            "state:{}\t"
            "joiner:{:x}\t"
            "detached:{}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{:x}\t"
            "ctx:{:x}\t"
            "owner:{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(id_.ptr)
        ,   static_cast<global_ult_state_underlying_t>(get_state())
        ,   reinterpret_cast<mgbase::uintptr_t>(get_joiner().ptr)
        ,   is_detached()
        ,   reinterpret_cast<mgbase::uintptr_t>(get_stack_ptr())
        ,   get_stack_size()
        ,   reinterpret_cast<mgbase::uintptr_t>(get_context().p)
        ,   get_owner_proc()
        );
        return w.str();
    }
    
private:
    global_ult_state get_state() const
    {
        return load_desc_member(&global_ult_desc::state);
    }
    
    template <typename T>
    T load_desc_member(T (global_ult_desc::*mem)) const
    {
        const auto target_proc = get_target_proc();
        const auto rptr = desc_rptr_.member(mem);
        
        const auto lptr = mgcom::rma::allocate<T>();
        
        mgcom::rma::read(target_proc, rptr, lptr, 1);
        
        const auto ret = *lptr;
        
        mgcom::rma::deallocate(lptr);
        
        return ret;
    }
    
    template <typename T>
    void store_desc_member(T (global_ult_desc::*mem), T val) const
    {
        const auto target_proc = get_target_proc();
        const auto rptr = desc_rptr_.member(mem);
        
        const auto lptr = mgcom::rma::allocate<T>();
        *lptr = val;
        
        mgcom::rma::write(target_proc, rptr, lptr, 1);
        
        mgcom::rma::deallocate(lptr);
    }
    
    void set_invalid()
    {
        id_ = mgult::make_invalid_ult_id();
    }
    
    mgcom::process_id_t get_target_proc() const MGBASE_NOEXCEPT
    {
        return id_.di.proc;
    }
    
    ult_id          id_;
    desc_remote_ptr desc_rptr_;
};

} // namespace mgth

