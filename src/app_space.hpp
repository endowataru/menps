
#pragma once

#include "app_space_protector.hpp"
#include "sharer/sharer_block_accessor.hpp"

namespace mgdsm {

class app_space
{
public:
    explicit app_space(app_space_protector& protector)
        : protector_(protector)
    { }
    
    bool fetch(sharer_block::accessor& blk_pr)
    {
        // Try to fetch the block.
        if (blk_pr.fetch())
        {
            // The block was marked as PROT_NONE.
            
            // Make the block readable.
            protector_.set_readonly(blk_pr);
            
            return true;
        }
        else
            return false;
    }
    
    bool touch(sharer_block::accessor& blk_pr)
    {
        // Try to touch the block.
        if (blk_pr.touch())
        {
            // The block was not marked as PROT_WRITE.
            
            // Make the block writable.
            protector_.set_writable(blk_pr);
            
            return true;
        }
        else
            return false;
    }
    
    bool reconcile(sharer_block::accessor& blk_pr)
    {
        // Check whether the block is dirty.
        if (blk_pr.is_reconcile_needed())
        {
            // Prohibit the block from being written by the other threads first.
            protector_.set_readonly(blk_pr);
            
            // Write back the dirty block to the backing store.
            blk_pr.reconcile();
            
            return true;
        }
        else
            return false;
    }
    
    bool flush(sharer_block::accessor& blk_pr)
    {
        if (blk_pr.is_flush_needed())
        {
            // Prohibit the block from being read by the other threads first.
            protector_.set_invalid(blk_pr);
            
            // Remove the cached block.
            blk_pr.flush();
            
            return true;
        }
        else
            return false;
    }
    
private:
    app_space_protector& protector_;
};

} // namespace mgdsm

