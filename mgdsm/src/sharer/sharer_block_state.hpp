
#pragma once

#include "mgdsm_common.hpp"
#include <mgbase/assert.hpp>

namespace mgdsm {

class sharer_block_state
{
    typedef mgbase::uint32_t integer_type;
    
    enum class state
        : integer_type
    {
        invalid = 0
    ,   clean
    ,   dirty
    ,   pinned
    };
    
    // Old GCC cannot compare scoped enums directly.
    static bool is_equal(const state a, const state b) {
        return static_cast<integer_type>(a) == static_cast<integer_type>(b);
    }
    static bool is_greater_than_or_equal(const state a, const state b) {
        return static_cast<integer_type>(a) >= static_cast<integer_type>(b);
    }
    
public:
    sharer_block_state() MGBASE_NOEXCEPT
        : st_()
    { }
    
    sharer_block_state(const sharer_block_state&) MGBASE_DEFAULT_NOEXCEPT = default;
    sharer_block_state& operator = (const sharer_block_state&) MGBASE_DEFAULT_NOEXCEPT = default;
    
    bool is_readable() const MGBASE_NOEXCEPT
    {
        return is_greater_than_or_equal(get_state(), state::clean);
    }
    void change_clean_to_invalid() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_equal(get_state(), state::clean));
        
        set_state(state::invalid);
    }
    void change_invalid_to_clean() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_equal(get_state(), state::invalid));
        
        set_state(state::clean);
    }
    void change_dirty_to_clean() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_equal(get_state(), state::dirty));
        
        set_state(state::clean);
    }
    
    bool is_writable() const MGBASE_NOEXCEPT
    {
        return is_greater_than_or_equal(get_state(), state::dirty);
    }
    void change_clean_to_dirty() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_equal(get_state(), state::clean));
        
        set_state(state::dirty);
    }
    void set_unpinned() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_pinned());
        
        set_state(state::dirty);
    }
    
    bool is_pinned() const MGBASE_NOEXCEPT
    {
        return is_equal(get_state(), state::pinned);
    }
    void set_pinned() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(is_equal(get_state(), state::dirty));
        
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

