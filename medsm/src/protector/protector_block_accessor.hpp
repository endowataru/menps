
#pragma once

#include "basic_protector_block_accessor.hpp"
#include "basic_block_protector.hpp"
#include "sharer/sharer_block_accessor.hpp"

namespace menps {
namespace medsm {

class protector_block_accessor;

struct protector_block_accessor_policy
{
    typedef protector_block_accessor    derived_type;
    
    typedef sharer_block::accessor      sharer_block_accessor_type;
};

class protector_segment;

class protector_block_accessor
    : public basic_protector_block_accessor<protector_block_accessor_policy>
    , public basic_block_protector<protector_block_accessor_policy>
{
    typedef basic_protector_block_accessor<protector_block_accessor_policy> base;
    
public:
    /*implicit*/ protector_block_accessor(
        protector_segment&      seg
    ,   sharer_block::accessor  sh_blk_ac
    )
        : seg_(seg)
        , sh_blk_ac_(mefdn::move(sh_blk_ac))
    { }
    
private:
    friend class basic_protector_block_accessor<protector_block_accessor_policy>;
    // friend base;
    
    friend class basic_block_protector<protector_block_accessor_policy>;
    
    sharer_block::accessor& get_sharer_block_accessor() noexcept {
        return sh_blk_ac_;
    }
    
    inline mefdn::size_t get_max_seg_size() const noexcept;
    inline void* get_segment_app_ptr() const noexcept;
    
    inline void add_new_read();
    inline void add_new_write();
    
    protector_segment&          seg_;
    sharer_block::accessor      sh_blk_ac_;
};

} // namespace medsm
} // namespace menps

