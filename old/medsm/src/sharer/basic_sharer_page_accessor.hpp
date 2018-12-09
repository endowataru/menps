
#pragma once

#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/memory/next_in_bytes.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/thread/spinlock.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_sharer_page_accessor
    : public mefdn::crtp_base<Policy>
{
    typedef typename Policy::derived_type           derived_type;
    typedef typename Policy::segment_accessor_type  segment_accessor_type;
    typedef typename Policy::page_entry_type        page_entry_type;
    typedef typename Policy::block_id_type          block_id_type;
    typedef typename Policy::prptr_type             prptr_type;
    typedef typename Policy::plptr_type             plptr_type;
    typedef typename Policy::difference_type        difference_type;
    typedef typename Policy::index_type             index_type;
    
    typedef mefdn::spinlock                        transfer_lock_type;
    typedef mefdn::unique_lock<transfer_lock_type> unique_transfer_lock_type;
    
public:
    // Transfer functions
    mefdn::size_t get_block_size() const noexcept
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Load the block size from the page entry.
        return pg_ent.get_block_size();
    }
    mefdn::size_t get_num_blocks() const noexcept
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.get_num_blocks();
    }
    
    unique_transfer_lock_type get_transfer_lock()
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.get_transfer_lock();
    }
    
    bool is_diff_needed(unique_transfer_lock_type& lk)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.is_diff_needed(lk);
    }
    bool is_flush_needed(unique_transfer_lock_type& lk)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return pg_ent.is_flush_needed(lk);
    }
    void enable_diff(unique_transfer_lock_type& lk)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        pg_ent.enable_diff(lk);
    }
    void enable_flush(unique_transfer_lock_type& lk)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        pg_ent.enable_flush(lk);
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
            
            // Allocate a new region if this is the first touch.
            if (this->create_page_if_invalid(seg_pr, &ret.owner_plptr))
            {
                // Set the new page location on the manager.
                seg_pr.assign_reader_page(pg_id, ret.owner_plptr);
            }
            
            {
                auto lk = pg_ent.get_transfer_lock();
                
                // Update the owner with the lookup result from the manager.
                pg_ent.update_owner_for_read(lk, mefdn::move(ret));
            }
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
            
            // Allocate a new region if this is the first touch or started migrating.
            if (this->create_page_if_invalid(seg_pr, &ret.owner_plptr))
            {
                // Set the new page location on the manager.
                seg_pr.assign_writer_page(pg_id, ret.owner_plptr);
            }
            
            {
                auto lk = pg_ent.get_transfer_lock();
                
                // Update the owner with the lookup result from the manager.
                pg_ent.update_owner_for_write(lk, mefdn::move(ret));
            }
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
            auto ret = seg_pr.release_write_page(pg_id);
            
            {
                auto lk = pg_ent.get_transfer_lock();
                
                // Update "needs_flush".
                pg_ent.update_for_release_write(lk, mefdn::move(ret));
            }
        }
    }
    
    void* get_block_sys_ptr(const block_id_type blk_id) const noexcept
    {
        auto& self = this->derived();
        auto& seg_pr = self.get_segment_accessor();
        const auto pg_id = self.get_page_id();
        
        const auto pg_ptr = seg_pr.get_page_sys_ptr(pg_id);
        
        const auto blk_offset = this->get_block_offset(blk_id);
        
        return mefdn::next_in_bytes(pg_ptr, blk_offset);
    }
    
    prptr_type get_read_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MEFDN_ASSERT(pg_ent.is_readable_from_owner());
        
        return this->get_owner_prptr(blk_id);
    }
    prptr_type get_write_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MEFDN_ASSERT(pg_ent.is_writable_from_owner());
        
        return this->get_owner_prptr(blk_id);
    }
    
    index_type get_index_in_segment() const noexcept
    {
        auto& self = this->derived();
        
        const auto pg_id = self.get_page_id();
        auto& seg_pr = self.get_segment_accessor();
        
        const auto pg_size = seg_pr.get_page_size();
        return pg_id * pg_size;
    }
    
private:
    bool create_page_if_invalid(segment_accessor_type& seg_ac, plptr_type* const owner_result)
    {
        if (Policy::is_invalid_plptr(*owner_result))
        {
            const auto pg_size = seg_ac.get_page_size();
            
            // Allocate a new page.
            *owner_result = Policy::allocate_page(pg_size);
            
            return true;
        }
        else
            return false;
    }
    
    prptr_type get_owner_prptr(const block_id_type blk_id)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        const auto blk_offset = this->get_block_offset(blk_id);
        
        const auto pg_prptr = pg_ent.get_owner_prptr();
        
        return Policy::next_in_bytes(pg_prptr, blk_offset);
    }
    difference_type get_block_offset(const block_id_type blk_id) const noexcept
    {
        const auto blk_size = this->get_block_size();
        
        return static_cast<difference_type>(blk_id * blk_size);
    }
};

} // namespace medsm
} // namespace menps

