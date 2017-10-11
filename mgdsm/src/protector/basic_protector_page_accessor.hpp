
#pragma once

#include <mgbase/crtp_base.hpp>

namespace mgdsm {

template <typename Policy>
class basic_protector_page_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::block_id_type  block_id_type;
    
public:
    template <typename Func>
    void do_for_all_blocks_in(const mgbase::uintptr_t idx_in_pg, const mgbase::size_t size_in_bytes, Func func)
    {
        auto& self = this->derived();
        
        const auto first_blk_info = this->get_block_pos(idx_in_pg);
        
        // The last block includes (pos + size - 1).
        const auto last_block_info = this->get_block_pos(idx_in_pg + size_in_bytes - 1);
        
        // Iterate over [first, last], not [first, last).
        for (block_id_type blk_id = first_blk_info.blk_id; blk_id <= last_block_info.blk_id; ++blk_id)
        {
            auto blk_ac = self.get_block_accessor(blk_id);
            
            func(blk_ac);
        }
    }
    
    template <typename Func>
    void do_for_block_at(const mgbase::uintptr_t idx_in_pg, Func&& func)
    {
        auto& self = this->derived();
        
        const auto blk_info = this->get_block_pos(idx_in_pg);
        
        auto blk_ac = self.get_block_accessor(blk_info.blk_id);
        
        mgbase::forward<Func>(func)(blk_ac);
    }
    
private:
    struct block_pos
    {
        block_id_type       blk_id;
        mgbase::uintptr_t   idx_in_blk;
    };
    
    block_pos get_block_pos(const mgbase::uintptr_t idx_in_pg) const
    {
        auto& self = this->derived();
        const auto blk_size = self.get_block_size();
        
        return {
            static_cast<block_id_type>(idx_in_pg / blk_size)
        ,   idx_in_pg % blk_size
        };
    }
};

} // namespace mgdsm

