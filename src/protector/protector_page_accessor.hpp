
#pragma once

#include "basic_protector_page_accessor.hpp"
#include "sharer/sharer_block_accessor.hpp"

namespace mgdsm {

class protector_page_accessor;

struct protector_page_accessor_policy
{
    typedef protector_page_accessor     derived_type;
    
    typedef block_id_t                  block_id_type;
};

class protector_segment;

class protector_page_accessor
    : public basic_protector_page_accessor<protector_page_accessor_policy>
{
    typedef basic_protector_page_accessor<protector_page_accessor_policy> base;
    
public:
    /*implicit*/ protector_page_accessor(
        protector_segment&      seg
    ,   sharer_page::accessor   sh_pg_ac
    )
        : seg_(seg)
        , sh_pg_ac_(mgbase::move(sh_pg_ac))
    { }
    
    protector_block_accessor get_block_accessor(const block_id_t blk_id)
    {
        return protector_block_accessor(
            seg_
        ,   sh_pg_ac_.get_block_accessor(blk_id)
        );
    }
    
private:
    friend class basic_protector_page_accessor<protector_page_accessor_policy>;
    // friend base;
    
    mgbase::size_t get_block_size() const MGBASE_NOEXCEPT {
        return sh_pg_ac_.get_block_size();
    }
    inline mgbase::size_t get_max_seg_size() const MGBASE_NOEXCEPT;
    
    protector_segment&      seg_;
    sharer_page::accessor   sh_pg_ac_;
};

} // namespace mgdsm

