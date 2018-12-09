
#pragma once

#include <menps/meth/common.hpp>
#include "global_ult_desc.hpp"

#include <menps/mefdn/mutex.hpp>
#include <string>
#include <menps/mefdn/logger.hpp>

#include <menps/mecom/rma/unique_local_ptr.hpp>

#include "base_ult.hpp"

namespace menps {
namespace meth {

class global_ult_desc_pool;

//#define METH_ENABLE_RELAXED_ULT_DESC

class global_ult_ref
{
    typedef mecom::rma::remote_ptr<global_ult_desc> desc_remote_ptr;
    
    typedef global_ult_ref  lock_type;
    
public:
    typedef mefdn::unique_lock<lock_type>  unique_lock_type;
    
    global_ult_ref() noexcept {
        set_invalid();
    }
    
    global_ult_ref(const ult_id& id, const desc_remote_ptr& desc_rptr)
        : id_(id)
        , desc_rptr_(desc_rptr)
    {
        MEFDN_ASSERT(mecom::valid_process_id(id.di.proc));
        
        desc_lptr_.reset(mecom::rma::allocate<global_ult_desc>());
        
        load_desc();
    }
    
    ~global_ult_ref() = default;
    
    global_ult_ref(const global_ult_ref&) = delete;
    global_ult_ref& operator = (const global_ult_ref&) = delete;
    
    global_ult_ref(global_ult_ref&& other) noexcept {
        set_invalid();
        *this = mefdn::move(other);
    }
    global_ult_ref& operator = (global_ult_ref&& other) noexcept {
        id_ = other.id_;
        desc_rptr_ = other.desc_rptr_;
        desc_lptr_ = mefdn::move(other.desc_lptr_);
        
        other.set_invalid();
        return *this;
    }
    
    void* get_stack_ptr() const noexcept
    {
        return load_desc_member_relaxed(&global_ult_desc::stack_ptr);
    }
    mefdn::size_t get_stack_size() const noexcept
    {
        return load_desc_member_relaxed(&global_ult_desc::stack_size);
    }
    
    template <typename... Args>
    unique_lock_type get_lock(Args&&... args)
    {
        mefdn::unique_lock<lock_type> lk(*this, mefdn::forward<Args>(args)...);
        return lk;
    }
    
    void lock()
    {
        while (!try_lock()) {
            base_ult::this_thread::yield();
        }
    }
    
    bool try_lock()
    {
        const auto target_proc = get_target_proc();
        const auto lock_rptr = desc_rptr_.member(&global_ult_desc::lock);
        
        MEFDN_LOG_VERBOSE(
            "msg:Trying to lock thread descriptor.\t"
            "{}"
        ,   to_string()
        );
        
        mecom::rma::atomic_default_t result{};
        
        mecom::rma::compare_and_swap<mecom::rma::atomic_default_t>(
            target_proc
        ,   lock_rptr
        ,   0
        ,   1
        ,   &result
        );
        
        const bool locked = result == 0;
        
        MEFDN_LOG_VERBOSE(
            "msg:{} thread descriptor.\t"
            "{}"
        ,   (locked ? "Locked" : "Failed to lock")
        ,   to_string()
        );
        
        if (locked) {
            load_desc();
            #ifdef METH_ENABLE_RELAXED_ULT_DESC
            MEFDN_ASSERT(desc_lptr_->lock == 1);
            #endif
        }
        
        return locked;
    }
    
    void unlock()
    {
        MEFDN_LOG_VERBOSE(
            "msg:Unlocking thread descriptor.\t"
            "{}"
        ,   to_string()
        );
        
        #ifdef METH_ENABLE_RELAXED_ULT_DESC
        MEFDN_ASSERT(desc_lptr_->lock == 1);
        #endif
        store_desc();
        
        const auto target_proc = get_target_proc();
        const auto lock_rptr = desc_rptr_.member(&global_ult_desc::lock);
        
        mecom::rma::atomic_write<mecom::rma::atomic_default_t>(
            target_proc
        ,   lock_rptr
        ,   0
        );
    }
    
    //mecom::process_id_t get_owner_proc(unique_lock_type& lk) {
    // FIXME: not locked
    mecom::process_id_t get_owner_proc() {
        //check_locked(lk);
        const auto proc = load_desc_member(&global_ult_desc::owner);
        return proc;
    }
    // FIXME: not locked
    void set_owner_proc(const mecom::process_id_t proc) {
        MEFDN_ASSERT(mecom::valid_process_id(proc));
        store_desc_member_relaxed(&global_ult_desc::owner, proc);
    }
    
    #if 0
    void set_blocked(unique_lock_type& lk) {
        check_locked(lk);
        MEFDN_ASSERT(get_state() == global_ult_state::ready);
        
        store_desc_member(&global_ult_desc::state, global_ult_state::blocked);
    }
    void set_ready(unique_lock_type& lk) {
        check_locked(lk);
        MEFDN_ASSERT(get_state() == global_ult_state::blocked);
        
        store_desc_member(&global_ult_desc::state, global_ult_state::ready);
    }
    #endif
    void set_finished(unique_lock_type& lk) {
        check_locked(lk);
        MEFDN_ASSERT(get_state() == global_ult_state::ready);
        
        store_desc_member_relaxed(&global_ult_desc::state, global_ult_state::finished);
    }
    // FIXME: not locked
    void invalidate_desc() {
        store_desc_member(&global_ult_desc::state, global_ult_state::invalid);
    }
    
    bool is_latest_stamp(unique_lock_type& lk) {
        #ifdef METH_ENABLE_ASYNC_WRITE_BACK
        check_locked(lk);
        
        auto cur_stamp = load_desc_member_relaxed(&global_ult_desc::cur_stamp);
        auto old_stamp = load_desc_member_relaxed(&global_ult_desc::old_stamp);
        
        MEFDN_ASSERT(cur_stamp - old_stamp >= 0);
        
        return cur_stamp == old_stamp;
        #else
        return true;
        #endif
    }
    
    bool is_finished(unique_lock_type& lk) const noexcept {
        check_locked(lk);
        return get_state() == global_ult_state::finished;
    }
    
    bool is_detached(unique_lock_type& lk) const noexcept {
        check_locked(lk);
        return load_desc_member_relaxed(&global_ult_desc::detached) == 1;
    }
    void set_detached(unique_lock_type& lk) const noexcept {
        check_locked(lk);
        const decltype(load_desc_member_relaxed(&global_ult_desc::detached)) one = 1;
        store_desc_member_relaxed(&global_ult_desc::detached, one);
    }
    
    bool has_joiner(unique_lock_type& lk) {
        check_locked(lk);
        return !is_invalid_ult_id(get_joiner(lk));
    }
    void set_joiner(unique_lock_type& lk, const global_ult_ref& joiner) {
        check_locked(lk);
        store_desc_member_relaxed(&global_ult_desc::joiner, joiner.get_id());
    }
    ult_id get_joiner(unique_lock_type& lk) {
        check_locked(lk);
        return load_desc_member_relaxed(&global_ult_desc::joiner);
    }
    
    bool is_valid() const noexcept {
        //return ! meult::is_invalid_ult_id(id_);
        if (! meult::is_invalid_ult_id(id_)) {
            MEFDN_ASSERT(get_state() != global_ult_state::invalid);
            return true;
        }
        else
            return false;
    }
    
    #ifdef METH_ENABLE_ASYNC_WRITE_BACK
private:
    struct do_update_stamp
    {
        global_ult_desc_pool&   pool; // TODO
        ult_id                  id;
        global_ult_stamp_t      stamp;
        
        // TODO : arrange dependency on global_ult_desc_pool
        inline void operator() () const;
    };
    
public:
    do_update_stamp make_update_stamp(unique_lock_type& lk, global_ult_desc_pool& pool)
    {
        check_locked(lk);
        
        // Increment cur_stamp in a relaxed manner.
        auto cur_stamp = load_desc_member_relaxed(&global_ult_desc::cur_stamp);
        
        #ifdef MEFDN_DEBUG
        const auto old_stamp = load_desc_member_relaxed(&global_ult_desc::old_stamp);
        MEFDN_ASSERT(cur_stamp - old_stamp >= 0);
        
        MEFDN_LOG_INFO(
            "msg:Increment current stamp for thread.\t"
            "old_stamp:{}\t"
            "cur_stamp:{}\t"
            "{}"
        ,   old_stamp
        ,   cur_stamp
        ,   this->to_string()
        );
        #endif
        
        ++cur_stamp;
        store_desc_member_relaxed(&global_ult_desc::cur_stamp, cur_stamp);
        
        return do_update_stamp{ pool, this->get_id(), cur_stamp };
    }
    #endif
    
    ult_id get_id() const noexcept {
        return id_;
    }
    
    std::string to_string()
    {
        MEFDN_ASSERT(is_valid());
        MEFDN_ASSERT(mecom::valid_process_id(id_.di.proc));
        
        fmt::MemoryWriter w;
        w.write(
            "id:{:x}\t"
            "state:{}\t"
            "joiner:{:x}\t"
            "detached:{}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{:x}\t"
            "owner:{}"
        ,   reinterpret_cast<mefdn::uintptr_t>(id_.ptr)
        ,   static_cast<global_ult_state_underlying_t>(get_state())
        ,   reinterpret_cast<mefdn::uintptr_t>(load_desc_member_relaxed(&global_ult_desc::joiner).ptr)
        ,   load_desc_member_relaxed(&global_ult_desc::detached)
        ,   reinterpret_cast<mefdn::uintptr_t>(get_stack_ptr())
        ,   get_stack_size()
        ,   load_desc_member_relaxed(&global_ult_desc::owner)
        );
        return w.str();
    }
    
private:
    global_ult_state get_state() const
    {
        return load_desc_member_relaxed(&global_ult_desc::state);
    }
    
    template <typename T>
    T load_desc_member_relaxed(T (global_ult_desc::* const mem)) const
    {
        #ifdef METH_ENABLE_RELAXED_ULT_DESC
            return desc_lptr_.get()->*mem;
        #else
            return load_desc_member(mem);
        #endif
    }
    template <typename T>
    void store_desc_member_relaxed(T (global_ult_desc::* const mem), const T val) const
    {
        #ifdef METH_ENABLE_RELAXED_ULT_DESC
            desc_lptr_.get()->*mem = val;
        #else
            store_desc_member(mem, val);
        #endif
    }
    
    template <typename T>
    T load_desc_member(T (global_ult_desc::* const mem)) const
    {
        const auto target_proc = get_target_proc();
        const auto rptr = desc_rptr_.member(mem);
        
        const auto lptr = mefdn::reinterpret_pointer_cast<T>(desc_lptr_.get());
        
        mecom::rma::read(target_proc, rptr, lptr, 1);
        
        const auto ret = *lptr;
        
        return ret;
    }
    template <typename T>
    void store_desc_member(T (global_ult_desc::* const mem), const T val) const
    {
        const auto target_proc = get_target_proc();
        const auto rptr = desc_rptr_.member(mem);
        
        const auto lptr = mefdn::reinterpret_pointer_cast<T>(desc_lptr_.get());
        *lptr = val;
        
        mecom::rma::write(target_proc, rptr, lptr, 1);
    }
    
    void set_invalid()
    {
        id_ = meult::make_invalid_ult_id();
    }
    
    mecom::process_id_t get_target_proc() const noexcept
    {
        return id_.di.proc;
    }
    
    void check_locked(unique_lock_type& lk) const {
        MEFDN_ASSERT(is_valid());
        MEFDN_ASSERT(lk.mutex() == this);
        MEFDN_ASSERT(lk.owns_lock());
    }
    
    void load_desc()
    {
        #ifdef METH_ENABLE_RELAXED_ULT_DESC
        const auto target_proc = get_target_proc();
        
        mecom::rma::read(
            target_proc
        ,   desc_rptr_
        ,   desc_lptr_.get()
        ,   1 // 1 element == sizeof(global_ult_desc)
        );
        #endif
    }
    void store_desc()
    {
        #ifdef METH_ENABLE_RELAXED_ULT_DESC
        const auto target_proc = get_target_proc();
        
        mecom::rma::write(
            target_proc
        ,   desc_rptr_
        ,   desc_lptr_.get()
        ,   1 // 1 element == sizeof(global_ult_desc)
        );
        #endif
    }
    
    ult_id          id_;
    desc_remote_ptr desc_rptr_;
    mecom::rma::unique_local_ptr<global_ult_desc>  desc_lptr_;
};

} // namespace meth
} // namespace menps

