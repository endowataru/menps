
#pragma once

#include <menps/mefdn/crtp_base.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_protector_block_accessor
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
public:
    bool fetch()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        // Try to fetch the block.
        const bool ret = sh_blk.fetch();
        if (ret) {
            // The block has been marked as PROT_NONE.
            
            // Make the block readable.
            self.set_readonly();
            
            // Add this block as a flushed page.
            self.add_new_read();
        }
        return ret;
    }
    
    bool touch()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        // Try to touch the block.
        const bool ret = sh_blk.touch();
        if (ret) {
            // The block has not been marked as PROT_WRITE.
            
            // Make the block writable.
            self.set_writable();
            
            // Add this block as a reconciled page.
            self.add_new_write();
        }
        return ret;
    }
    
    bool reconcile()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        // Check whether the block is dirty.
        const bool ret = sh_blk.is_reconcile_needed();
        if (ret) {
            // Prohibit the block from being written by the other threads first.
            self.set_readonly();
            
            // Write back the dirty block to the backing store.
            sh_blk.reconcile();
        }
        return ret;
    }
    
    bool flush()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        // Check whether the block is valid.
        const bool ret = sh_blk.is_flush_needed();
        if (ret) {
            // Prohibit the block from being read by the other threads first.
            self.set_invalid();
            
            // Remove the cached block.
            sh_blk.flush();
        }
            
        return ret;
    }
    
    void pin()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        sh_blk.pin();
    }
    
    void unpin()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        sh_blk.unpin();
        
        // This block must be marked as a candidate
        // for both flushed and reconciled pages
        // because it is now in the "dirty" state.
        self.add_new_read();
        self.add_new_write();
    }
};

} // namespace medsm
} // namespace menps

