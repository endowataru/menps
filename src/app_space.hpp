
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
    
    bool fetch(sharer_block::accessor& blk_ac)
    {
        // Try to fetch the block.
        if (blk_ac.fetch())
        {
            // The block was marked as PROT_NONE.
            
            // Make the block readable.
            protector_.set_readonly(blk_ac);
            
            return true;
        }
        else
            return false;
    }
    
    bool touch(sharer_block::accessor& blk_ac)
    {
        // Try to touch the block.
        if (blk_ac.touch())
        {
            // The block was not marked as PROT_WRITE.
            
            // Make the block writable.
            protector_.set_writable(blk_ac);
            
            return true;
        }
        else
            return false;
    }
    
    bool reconcile(sharer_block::accessor& blk_ac)
    {
        // Check whether the block is dirty.
        if (blk_ac.is_reconcile_needed())
        {
            // Prohibit the block from being written by the other threads first.
            protector_.set_readonly(blk_ac);
            
            // Write back the dirty block to the backing store.
            blk_ac.reconcile();
            
            return true;
        }
        else
            return false;
    }
    
    bool flush(sharer_block::accessor& blk_ac)
    {
        if (blk_ac.is_flush_needed())
        {
            // Prohibit the block from being read by the other threads first.
            protector_.set_invalid(blk_ac);
            
            // Remove the cached block.
            blk_ac.flush();
            
            return true;
        }
        else
            return false;
    }
    
    void pin(sharer_block::accessor& blk_ac)
    {
        blk_ac.pin();
    }
    void unpin(sharer_block::accessor& blk_ac)
    {
        blk_ac.unpin();
    }
    
private:
    app_space_protector& protector_;
};

} // namespace mgdsm

