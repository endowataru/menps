
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
    
    /*template <typename Func>
    void for_all_blocks(void* const ptr, const mgbase::size_t size_in_bytes, Func func);*/
    /*{
        const auto seg_id = get_segment_id(ptr);
        
        auto seg_pr = this->space_->get_segment_accessor(seg_id);
        
        const auto first_pg_id = get_first_page_id(seg_pr, ptr, size_in_bytes);
        const auto last_pg_id = get_last_page_id(seg_pr, ptr, size_in_bytes);
        
        for (page_id_t pg_id = first_pg_id; pg_id <= last_pg_id; ++pg_id)
        {
            auto pg_pr = seg_pr.get_page_accessor(pg_id);
            
            const auto first_blk_id = get_first_block_id(seg_pr, pg_pr, ptr, size_in_bytes);
            const auto last_blk_id = get_last_block_id(seg_pr, pg_pr, ptr, size_in_bytes);
            
            for (block_id_t blk_id = first_blk_id; blk_id <= last_blk_id; ++blk_id)
            {
                auto blk_pr = pg_pr.get_block_accessor(blk_id);
                
                func(blk_pr);
            }
        }
    }*/
    
    template <typename Func>
    void do_for_block_at(Func&& func, void* ptr)
    {
        // Find a segment.
        
        const auto abs_idx = get_index_from_app_ptr(ptr);
        
        const auto max_seg_size = get_max_segment_size();
        
        const auto seg_id =
            static_cast<segment_id_t>(abs_idx / max_seg_size);
        
        const auto idx_in_seg = abs_idx % max_seg_size;
        
        auto seg_pr = this->conf_.space.get_segment_accessor(seg_id);
        
        // Find a page.
        
        const auto pg_size = seg_pr.get_page_size();
        
        const auto pg_id =
            static_cast<page_id_t>(idx_in_seg / pg_size);
        
        const auto idx_in_pg = idx_in_seg % pg_size;
        
        auto pg_pr = seg_pr.get_page_accessor(pg_id);
        
        // Find a block.
        
        const auto blk_size = pg_pr.get_block_size();
        
        const auto blk_id =
            static_cast<block_id_t>(idx_in_pg / blk_size);
        
        auto blk_ac = pg_pr.get_block_accessor(blk_id);
        
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

