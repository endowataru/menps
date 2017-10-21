
#pragma once

#include "basic_protector_page_accessor.hpp"
#include "sharer/sharer_block_accessor.hpp"

namespace menps {
namespace medsm {

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
        : seg_(&seg)
        , sh_pg_ac_(mefdn::move(sh_pg_ac))
    { }
    
    protector_page_accessor(const protector_page_accessor&) = delete;
    protector_page_accessor& operator = (const protector_page_accessor&) = delete;
    
    protector_page_accessor(protector_page_accessor&&) noexcept = default;
    protector_page_accessor& operator = (protector_page_accessor&&) noexcept = default;
    
    protector_block_accessor get_block_accessor(const block_id_t blk_id)
    {
        return protector_block_accessor(
            *seg_
        ,   sh_pg_ac_.get_block_accessor(blk_id)
        );
    }
    
private:
    friend class basic_protector_page_accessor<protector_page_accessor_policy>;
    // friend base;
    
    mefdn::size_t get_block_size() const noexcept {
        return sh_pg_ac_.get_block_size();
    }
    inline mefdn::size_t get_max_seg_size() const noexcept;
    
    protector_segment*      seg_;
    sharer_page::accessor   sh_pg_ac_;
};

} // namespace medsm
} // namespace menps

