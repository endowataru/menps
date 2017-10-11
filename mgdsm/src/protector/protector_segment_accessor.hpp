
#pragma once

#include "basic_protector_segment_accessor.hpp"
#include "protector_block_accessor.hpp"
#include "protector_page_accessor.hpp"
#include "sharer/sharer_block_accessor.hpp"

namespace mgdsm {

class protector_segment_accessor;

struct protector_segment_accessor_policy
{
    typedef protector_segment_accessor  derived_type;
    typedef page_id_t                   page_id_type;
    typedef protector_block_accessor    block_accessor_type;
};

class protector_segment;

class protector_segment_accessor
    : public basic_protector_segment_accessor<protector_segment_accessor_policy>
{
    typedef basic_protector_segment_accessor<protector_segment_accessor_policy> base;
    
public:
    /*implicit*/ protector_segment_accessor(
        protector_segment&          seg
    ,   sharer_segment::accessor    sh_seg_ac
    )
        : seg_(seg)
        , sh_seg_ac_(mgbase::move(sh_seg_ac))
    { }
    
    protector_page_accessor get_page_accessor(const page_id_t pg_id)
    {
        return protector_page_accessor(
            seg_
        ,   sh_seg_ac_.get_page_accessor(pg_id)
        );
    }
    
private:
    friend class basic_protector_segment_accessor<protector_segment_accessor_policy>;
    // friend base;
    
    mgbase::size_t get_page_size() const MGBASE_NOEXCEPT {
        return sh_seg_ac_.get_page_size();
    }
    
    protector_segment&          seg_;
    sharer_segment::accessor    sh_seg_ac_;
};

} // namespace mgdsm

