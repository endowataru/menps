
#pragma once

#include "mgdsm_common.hpp"
#include <mgbase/assert.hpp>

namespace mgdsm {

class sharer_block_state
{
    enum class state
    {
        invalid = 0
    ,   clean
    ,   dirty
    ,   pinned
    };
    
public:
    sharer_block_state() MGBASE_NOEXCEPT
        : st_()
    { }
    
    sharer_block_state(const sharer_block_state&) MGBASE_DEFAULT_NOEXCEPT = default;
    sharer_block_state& operator = (const sharer_block_state&) MGBASE_DEFAULT_NOEXCEPT = default;
    
    bool is_readable() const MGBASE_NOEXCEPT
    {
        return get_state() >= state::clean;
    }
    void change_clean_to_invalid() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(get_state() == state::clean);
        
        set_state(state::invalid);
    }
    void change_invalid_to_clean() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(get_state() == state::invalid);
        
        set_state(state::clean);
    }
    void change_dirty_to_clean() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(get_state() == state::dirty);
        
        set_state(state::clean);
    }
    
    bool is_writable() const MGBASE_NOEXCEPT
    {
        return get_state() >= state::dirty;
    }
    void change_clean_to_dirty() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(get_state() == state::clean);
        
        set_state(state::dirty);
    }
    void set_unpinned() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_pinned());
        
        set_state(state::dirty);
    }
    
    bool is_pinned() const MGBASE_NOEXCEPT
    {
        return get_state() == state::pinned;
    }
    void set_pinned() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(get_state() == state::dirty);
        
        set_state(state::pinned);
    }
    
private:
    state get_state() const MGBASE_NOEXCEPT {
        return st_;
    }
    void set_state(const state st) MGBASE_NOEXCEPT {
        st_ = st;
    }
    
    state st_;
};

} // namespace mgdsm

