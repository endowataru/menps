
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/assert.hpp>
#include <cstring>
#include <stdexcept>
#include <mgbase/logger.hpp>

namespace mgdsm {

template <typename Policy>
class basic_sharer_block_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    
    typedef typename Policy::index_type         index_type;
    
public:
    bool fetch()
    {
        // mprotect(PROT_READ) must be called after this function.
        
        auto& self = this->derived();
        
        auto& pg = self.get_page_accessor();
        auto& blk = self.get_block_entry();
        const auto blk_id = self.get_block_id();
        
        // Check whether the page is "readable" (= "clean"/"dirty"/"pinned").
        if (blk.is_readable()) {
            // This block became PROT_NONE even though the page was "valid".
            // This happens when the epoch proceeded in order to update the access history.
            
            // The fetch operation didn't happen.
            return false;
        }
        
        // This block was "invalid" and is now being fetched.
        
        const auto blk_size = pg.get_block_size();
        
        // Start reading this block.
        // The reading count is incremented because this page is becoming "valid".
        // If the page entry is not cached, the manager process will return
        // the owner process ID and the page address,
        // which will be returned in this function.
        pg.acquire_read_block(blk_id);
        
        // Get the address of the owner.
        const auto src_prptr = pg.get_read_owner_prptr(blk_id);
        
        // Get the local address of the twin.
        const auto twin_lptr = blk.get_twin_lptr(blk_size);
        
        // Get the pointer to the local cache block for the system (not for the application).
        const auto dest_ptr = pg.get_block_sys_ptr(blk_id);
        
        // Copy the whole page to the twin.
        // This is a typical behavior of home-based DSM.
        self.read(src_prptr, twin_lptr, dest_ptr, blk_size);
        
        // Mark this page as clean.
        // It is still disabled to write on this page.
        blk.change_invalid_to_clean();
        
        MGBASE_LOG_INFO(
            "msg:Fetched block.\t"
            "{}"
        ,   self.to_string()
        );
        
        // The data transfer happened.
        // Call mprotect(PROT_READ) later.
        return true;
    }
    
    bool touch()
    {
        // mprotect(PROT_READ|PROT_WRITE) must be called after this function
        // if this function returns "true".
        
        auto& self = this->derived();
        
        auto& pg = self.get_page_accessor();
        auto& blk = self.get_block_entry();
        const auto blk_id = self.get_block_id();
        
        // The page must be readable.
        // Invalid blocks must be fetched before calling touch.
        MGBASE_ASSERT(blk.is_readable());
        
        // Check whether the page is "writable" (= "dirty"/"pinned").
        if (blk.is_writable()) {
            // This block didn't have PROT_WRITE even though the page was "valid".
            // The same reason in fetch applies.
            
            // The touch operation didn't happen.
            return false;
        }
        
        // This block was not "dirty" and is now being touched.
        
        // Start writing this block.
        pg.acquire_write_block(blk_id);
        
        // Mark this page as "dirty".
        blk.change_clean_to_dirty();
        
        MGBASE_LOG_INFO(
            "msg:Touched block.\t"
            "{}"
        ,   self.to_string()
        );
        
        // Call mprotect(PROT_WRITE) later.
        return true;
    }
    
    bool is_reconcile_needed()
    {
        auto& self = this->derived();
        
        auto& blk = self.get_block_entry();
        
        // The current implementation always requires reconcile for all of dirty pages.
        return blk.is_writable()
        // Pinned pages must not be reconciled (= mprotect(PROT_READ).)
            && ! blk.is_pinned();
        
        // TODO: Should private pages be reconciled?
    }
    
    void reconcile()
    {
        // mprotect(PROT_READ) must be called before.
        // (lock -> mprotect -> reconcile -> unlock)
        
        auto& self = this->derived();
        
        auto& pg = self.get_page_accessor();
        auto& blk = self.get_block_entry();
        const auto blk_id = self.get_block_id();
        
        // The page must be being written.
        MGBASE_ASSERT(is_reconcile_needed());
        
        //MGBASE_ASSERT(pg.is_reconcile_needed());
        
        const auto blk_size = pg.get_block_size();
        
        // Get the pointer to the local cache block for the system (not for the application).
        const auto src_ptr = pg.get_block_sys_ptr(blk_id);
        
        // Get the local address of the twin.
        const auto twin_lptr = blk.get_twin_lptr(blk_size);
        
        // Get the address of the owner's block.
        const auto dest_prptr = pg.get_write_owner_prptr(blk_id);
        
        // Check whether the page is only written by this process.
        if (pg.is_diff_needed()) {
            // Write the whole page.
            self.write_whole(src_ptr, twin_lptr, dest_prptr, blk_size);
        }
        else {
            // Write diffs because there are multiple writers.
            self.write_diffs(src_ptr, twin_lptr, dest_prptr, blk_size);
        }
        
        // Mark this block as clean.
        blk.change_dirty_to_clean();
        
        // Decrement the count of writing blocks for this page.
        // If it becomes 0, then a message will be sent to the manager process.
        pg.release_write_block(blk_id);
        
        MGBASE_LOG_INFO(
            "msg:Reconciled block.\t"
            "{}"
        ,   self.to_string()
        );
    }
    
    bool is_flush_needed()
    {
        auto& self = this->derived();
        
        auto& blk = self.get_block_entry();
        
        // Invalid blocks need not to be flushed.
        return blk.is_readable()
        // Pinned block must not be flushed (= mprotect(PROT_NONE).)
            && ! blk.is_pinned();
    }
    
    void flush()
    {
        // mprotect(PROT_NONE) must be called before.
        // (lock -> mprotect -> flush -> unlock)
        
        auto& self = this->derived();
        
        auto& pg = self.get_page_accessor();
        auto& blk = self.get_block_entry();
        const auto blk_id = self.get_block_id();
        
        // The block must be readable.
        MGBASE_ASSERT(this->is_flush_needed());
        
        // Mark this block as invalid.
        blk.change_clean_to_invalid();
        
        // Decrement the count of reading blocks for this page.
        // If necessary, it will tell the manager process
        // to abandon the privilege as a reader process.
        pg.release_read_block(blk_id);
        
        MGBASE_LOG_INFO(
            "msg:Flushed block.\t"
            "{}"
        ,   self.to_string()
        );
    }
    
    void pin()
    {
        auto& self = this->derived();
        
        auto& blk = self.get_block_entry();
        
        // Only unpinned blocks are pinned.
        if (! blk.is_pinned())
        {
            // Mark this block as pinned.
            blk.set_pinned();
        }
    }
    
    void unpin()
    {
        auto& self = this->derived();
        
        auto& blk = self.get_block_entry();
        
        // Only pinned blocks are pinned.
        if (blk.is_pinned())
        {
            // Mark this block as dirty (not pinned).
            blk.set_unpinned();
        }
    }
    
    mgbase::size_t get_block_size() MGBASE_NOEXCEPT
    {
        auto& self = this->derived();
        auto& pg_pr = self.get_page_accessor();
        
        return pg_pr.get_block_size();
    }
    
    index_type get_index_in_segment()
    {
        auto& self = this->derived();
        auto& pg_pr = self.get_page_accessor();
        
        const auto pg_idx = pg_pr.get_index_in_segment();
        const auto blk_size = pg_pr.get_block_size();
        
        const auto blk_id = self.get_block_id();
        
        return pg_idx + blk_id * blk_size;
    }
};

} // namespace mgdsm

