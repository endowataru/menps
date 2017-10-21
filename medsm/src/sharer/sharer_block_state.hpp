
#pragma once

#include "medsm_common.hpp"
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm {

class sharer_block_state
{
    typedef mefdn::uint32_t integer_type;
    
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
    sharer_block_state() noexcept
        : st_()
    { }
    
    sharer_block_state(const sharer_block_state&) noexcept = default;
    sharer_block_state& operator = (const sharer_block_state&) noexcept = default;
    
    bool is_readable() const noexcept
    {
        return is_greater_than_or_equal(get_state(), state::clean);
    }
    void change_clean_to_invalid() noexcept
    {
        MEFDN_ASSERT(is_equal(get_state(), state::clean));
        
        set_state(state::invalid);
    }
    void change_invalid_to_clean() noexcept
    {
        MEFDN_ASSERT(is_equal(get_state(), state::invalid));
        
        set_state(state::clean);
    }
    void change_dirty_to_clean() noexcept
    {
        MEFDN_ASSERT(is_equal(get_state(), state::dirty));
        
        set_state(state::clean);
    }
    
    bool is_writable() const noexcept
    {
        return is_greater_than_or_equal(get_state(), state::dirty);
    }
    void change_clean_to_dirty() noexcept
    {
        MEFDN_ASSERT(is_equal(get_state(), state::clean));
        
        set_state(state::dirty);
    }
    void set_unpinned() noexcept
    {
        MEFDN_ASSERT(is_pinned());
        
        set_state(state::dirty);
    }
    
    bool is_pinned() const noexcept
    {
        return is_equal(get_state(), state::pinned);
    }
    void set_pinned() noexcept
    {
        MEFDN_ASSERT(is_equal(get_state(), state::dirty));
        
        set_state(state::pinned);
    }
    
private:
    state get_state() const noexcept {
        return st_;
    }
    void set_state(const state st) noexcept {
        st_ = st;
    }
    
    state st_;
};

} // namespace medsm
} // namespace menps

