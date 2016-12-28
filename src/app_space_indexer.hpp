
#pragma once

#include "app_space.hpp"
#include "sharer/sharer_space.hpp"
#include <mgbase/memory/distance_in_bytes.hpp>

namespace mgdsm {

class app_space_indexer
{
public:
    struct config
    {
        sharer_space&   space;
        void*           app_ptr;
        mgbase::size_t  space_size;
        mgbase::size_t  seg_size;
    };
    
    explicit app_space_indexer(const config& conf)
        : conf_(conf)
    { }
    
    template <typename Func>
    void do_for_all_blocks_in(void* const ptr, const mgbase::size_t size_in_bytes, Func func)
    {
        const auto seg_info = this->get_segment_pos(ptr);
        
        const auto last_ptr = mgbase::next_in_bytes(ptr, size_in_bytes);
        const auto last_seg_info = this->get_segment_pos(last_ptr);
        
        MGBASE_ASSERT(seg_info.seg_id == last_seg_info.seg_id);
        
        auto seg_ac = this->conf_.space.get_segment_accessor(seg_info.seg_id);
        
        const auto first_pg_info = this->get_page_pos(seg_ac, seg_info.idx_in_seg);
        
        // The last page includes (pos + size - 1).
        const auto last_pg_info = this->get_page_pos(seg_ac, seg_info.idx_in_seg + size_in_bytes - 1);
        
        // Iterate over [first, last], not [first, last).
        for (page_id_t pg_id = first_pg_info.pg_id; pg_id <= last_pg_info.pg_id; ++pg_id)
        {
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            const auto num_blks = pg_ac.get_num_blocks();
            
            for (block_id_t blk_id = 0; blk_id < num_blks; ++blk_id)
            {
                auto blk_ac = pg_ac.get_block_accessor(blk_id);
                
                func(blk_ac);
            }
        }
    }
    
    template <typename Func>
    void do_for_block_at(Func&& func, void* ptr)
    {
        // Find a segment.
        
        const auto seg_info = this->get_segment_pos(ptr);
        
        auto seg_ac = this->conf_.space.get_segment_accessor(seg_info.seg_id);
        
        // Find a page.
        
        const auto pg_info = this->get_page_pos(seg_ac, seg_info.idx_in_seg);
        
        auto pg_ac = seg_ac.get_page_accessor(pg_info.pg_id);
        
        // Find a block.
        
        const auto blk_info = this->get_block_pos(pg_ac, pg_info.idx_in_pg);
        
        auto blk_ac = pg_ac.get_block_accessor(blk_info.blk_id);
        
        mgbase::forward<Func>(func)(blk_ac);
    }
    
    bool in_range(void* const ptr)
    {
        const auto first = this->conf_.app_ptr;
        const auto last =
            mgbase::next_in_bytes(first, get_address_space_size());
        
        return first <= ptr && ptr < last;
    }
    
private:
    struct segment_pos
    {
        segment_id_t        seg_id;
        mgbase::uintptr_t   idx_in_seg;
    };
    
    segment_pos get_segment_pos(void* const ptr)
    {
        const auto abs_idx = get_index_from_app_ptr(ptr);
        
        const auto max_seg_size = this->get_max_segment_size();
        
        return {
            static_cast<segment_id_t>(abs_idx / max_seg_size)
        ,   abs_idx % max_seg_size
        };
    }
    
    struct page_pos
    {
        page_id_t           pg_id;
        mgbase::uintptr_t   idx_in_pg;
    };
    
    static page_pos get_page_pos(sharer_segment::accessor& seg_ac, const mgbase::uintptr_t idx_in_seg)
    {
        const auto pg_size = seg_ac.get_page_size();
        
        return {
            static_cast<page_id_t>(idx_in_seg / pg_size)
        ,   idx_in_seg % pg_size
        };
    }
    
    struct block_pos
    {
        block_id_t          blk_id;
        mgbase::uintptr_t   idx_in_blk;
    };
    
    static block_pos get_block_pos(sharer_page::accessor& pg_ac, const mgbase::uintptr_t idx_in_pg)
    {
        const auto blk_size = pg_ac.get_block_size();
        
        return {
            static_cast<block_id_t>(idx_in_pg / blk_size)
        ,   idx_in_pg % blk_size
        };
    }
    
    mgbase::size_t get_max_segment_size()
    {
        return this->conf_.seg_size;
    }
    
    mgbase::size_t get_address_space_size()
    {
        return this->conf_.space_size;
    }
    
    mgbase::uintptr_t get_index_from_app_ptr(void* const ptr)
    {
        return mgbase::distance_in_bytes(this->conf_.app_ptr, ptr);
    }
    
    const config conf_;
};

} // namespace mgdsm

