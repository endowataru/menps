
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/memory/next_in_bytes.hpp>

namespace mgdsm {

template <typename Policy>
class basic_sharer_page_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    typedef typename Policy::segment_accessor_type segment_accessor_type;
    typedef typename Policy::page_entry_type    page_entry_type;
    typedef typename Policy::block_id_type      block_id_type;
    typedef typename Policy::prptr_type         prptr_type;
    typedef typename Policy::difference_type    difference_type;
    typedef typename Policy::index_type         index_type;
    
public:
    // Transfer functions
    mgbase::size_t get_block_size() const MGBASE_NOEXCEPT
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Load the block size from the page entry.
        return pg_ent.get_block_size();
    }
    bool is_diff_needed()
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.is_diff_needed();
    }
    bool is_flush_needed()
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.is_flush_needed();
    }
    void enable_diff()
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        pg_ent.enable_diff();
    }
    void enable_flush()
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        pg_ent.enable_flush();
    }
    
    // Called by fetch.
    void acquire_read_block(const block_id_type /*blk_id*/)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Add the block as a reader.
        if (pg_ent.add_read_block())
        {
            // This block is the first block that started reading in this page. 
            
            auto& seg_pr = self.get_segment_accessor();
            const auto pg_id = self.get_page_id();
            
            // Acquire the read privilege.
            auto ret = seg_pr.acquire_read_page(pg_id);
            
            // Update the owner with the lookup result from the manager.
            pg_ent.update_owner_for_read(mgbase::move(ret));
        }
    }
    
    // Called by flush.
    void release_read_block(const block_id_type /*blk_id*/)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Remove the block as a reader.
        if (pg_ent.remove_read_block())
        {
            // This block is the last block that finished reading in this page.
            
            auto& seg_pr = self.get_segment_accessor();
            const auto pg_id = self.get_page_id();
            
            // Release the read privilege.
            seg_pr.release_read_page(pg_id);
        }
    }
    
    // Called by touch.
    void acquire_write_block(const block_id_type /*blk_id*/)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Add the block as a writer.
        if (pg_ent.add_write_block())
        {
            // This block is the first block that started writing in this page.
            
            auto& seg_pr = self.get_segment_accessor();
            const auto pg_id = self.get_page_id();
            
            // Acquire the write privilege.
            auto ret = seg_pr.acquire_write_page(pg_id);
            
            // Update the owner with the lookup result from the manager.
            pg_ent.update_owner_for_write(mgbase::move(ret));
        }
    }
    
    // Called by reconcile.
    void release_write_block(const block_id_type /*blk_id*/)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Remove the block as a writer.
        if (pg_ent.remove_write_block())
        {
            // This block is the last block that finished writing in this page.
            
            auto& seg_pr = self.get_segment_accessor();
            const auto pg_id = self.get_page_id();
            
            // Release the write privilege.
            seg_pr.release_write_page(pg_id);
        }
    }
    
    void* get_block_sys_ptr(const block_id_type blk_id) const MGBASE_NOEXCEPT
    {
        auto& self = this->derived();
        auto& seg_pr = self.get_segment_accessor();
        const auto pg_id = self.get_page_id();
        
        const auto pg_ptr = seg_pr.get_page_sys_ptr(pg_id);
        
        const auto blk_offset = this->get_block_offset(blk_id);
        
        return mgbase::next_in_bytes(pg_ptr, blk_offset);
    }
    
    prptr_type get_read_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MGBASE_ASSERT(pg_ent.is_readable_from_owner());
        
        return this->get_owner_prptr(blk_id);
    }
    prptr_type get_write_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MGBASE_ASSERT(pg_ent.is_writable_from_owner());
        
        return this->get_owner_prptr(blk_id);
    }
    
    index_type get_index_in_segment() const MGBASE_NOEXCEPT
    {
        auto& self = this->derived();
        
        const auto pg_id = self.get_page_id();
        auto& seg_pr = self.get_segment_accessor();
        
        const auto pg_size = seg_pr.get_page_size();
        return pg_id * pg_size;
    }
    
private:
    prptr_type get_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        const auto blk_offset = this->get_block_offset(blk_id);
        
        const auto pg_prptr = pg_ent.get_owner_prptr();
        
        return Policy::next_in_bytes(pg_prptr, blk_offset);
    }
    difference_type get_block_offset(const block_id_type blk_id) const MGBASE_NOEXCEPT
    {
        const auto blk_size = this->get_block_size();
        
        return static_cast<difference_type>(blk_id * blk_size);
    }
};

} // namespace mgdsm

