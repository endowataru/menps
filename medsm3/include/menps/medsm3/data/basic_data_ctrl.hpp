
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_data_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using byte = fdn::byte;

    using merge_policy_type = typename P::merge_policy_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_base_type = typename P::blk_global_lock_base_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;

public:
    void fast_read(blk_local_lock_type& blk_llk, const proc_id_type wr_proc, const bool is_dirty)
    {
        auto& self = this->derived();
        const auto blk_size = blk_llk.blk_size();

        auto& com = blk_llk.get_com_itf();
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        if (!self.is_migration_enabled() || this_proc != wr_proc)
        {
            const auto local_working = self.get_local_working_lptr(blk_llk);
            const auto wr_source_rptr = self.get_remote_source_rptr(wr_proc, blk_llk);

            if (is_dirty) {
                CMPTH_P_PROF_SCOPE(P, merge_read);
                
                // Merge the diff in the read.
                
                // Read the data of the home process into a temporary buffer.
                const auto wr_source_buf = rma.buf_read(wr_proc, wr_source_rptr, blk_size);
                const auto wr_source = wr_source_buf.get();
                
                const auto local_snapshot = self.get_local_snapshot_ptr(blk_llk);
                
                // Apply the changes written in the home into local_working and local_snapshot.
                merge_policy_type::read_merge(blk_llk, wr_source, local_working, local_snapshot, blk_size);
            }
            else {
                CMPTH_P_PROF_SCOPE(P, wn_read);
                
                // Simply read from the home process.
                rma.read(wr_proc, wr_source_rptr, local_working, blk_size);
            }
        }
        
        {
            CMPTH_P_PROF_SCOPE(P, mprotect_start_read);
            
            // Call mprotect(PROT_READ).
            self.protect_readonly(blk_llk);
        }
    }

    void start_write(blk_local_lock_type& blk_llk, const bool needs_twin)
    {
        auto& self = this->derived();
        const auto blk_size = blk_llk.blk_size();
        
        const auto local_working = self.get_local_working_lptr(blk_llk);
        const auto local_snapshot = self.get_local_snapshot_ptr(blk_llk);
        
        if (needs_twin) {
            CMPTH_P_PROF_SCOPE(P, start_write_twin);
            
            // Copy the private data to the public data.
            // This is a preparation for releasing this block later.
            std::memcpy(local_snapshot, local_working, blk_size);
            //std::copy(local_working, local_working + blk_size, local_snapshot);
        }
        
        {
            CMPTH_P_PROF_SCOPE(P, mprotect_start_write);
            
            // Call mprotect(PROT_READ | PROT_WRITE).
            self.protect_writable(blk_llk);
        }
    }

    struct update_merge_result {
        // This block was written by this process
        // and must be recorded to the write notices.
        bool is_written;
        // This block is migrated from the previous owner.
        // If true, only this process has the latest data.
        // If false, the latest data is also written to the previous owner.
        bool is_migrated;
    };

    template <typename BeginRet>
    update_merge_result update_merge(blk_global_lock_base_type& blk_glk, const BeginRet& begin_ret)
    {
        CMPTH_P_PROF_SCOPE(P, tx_merge);
        
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();
        auto& com = blk_llk.get_com_itf();
        auto& rma = com.get_rma();
        const auto blk_size = blk_llk.blk_size();
        const auto migration_enabled = self.is_migration_enabled();
        
        if (begin_ret.needs_protect_before) {
            // Only when the block was writable and should be protected,
            // this method write-protects this block
            // in order to apply the changes to the private data.
            
            CMPTH_P_PROF_SCOPE(P, mprotect_tx_before);
            
            // Call mprotect(PROT_READ).
            self.protect_readonly(blk_llk);
        }
        
        const auto local_working = self.get_local_working_lptr(blk_llk);
        const auto local_snapshot = self.get_local_snapshot_ptr(blk_llk);
        
        bool is_written = false;
        
        if (begin_ret.is_dirty) {
            // Compare the private data with the public data.
            // Note that the private data is STILL WRITABLE
            // and can be modified concurrently by other threads in this process.
            // It's OK to read the intermediate states
            // because those writes will be managed by the next release operation.
            if (begin_ret.needs_local_comp){
                CMPTH_P_PROF_SCOPE(P, tx_merge_memcmp);
                
                is_written = std::memcmp(local_working, local_snapshot, blk_size) != 0;
                //std::equal(local_working, local_working + blk_size, local_snapshot)
            }
            else {
                is_written = true;
            }
        }
        
        const auto owner_proc = blk_glk.prev_owner();
        const auto owner_source_rptr = self.get_remote_source_rptr(owner_proc, blk_llk);
        
        if (! begin_ret.is_remotely_updated) {
            if (is_written) {
                if (begin_ret.needs_local_copy) {
                    CMPTH_P_PROF_SCOPE(P, tx_merge_local_memcpy);
                    
                    // Copy the private data to the public data.
                    std::memcpy(local_snapshot, local_working, blk_size);
                    //std::copy(local_working, local_working + blk_size, local_snapshot);
                }
            }
            else {
                // The data is not modified.
            }
        }
        else {
            typename rma_itf_type::template unique_local_ptr<byte []> owner_source_buf;
            {
                CMPTH_P_PROF_SCOPE(P, tx_merge_remote_get);

                // Read the data from owner_proc.
                owner_source_buf = rma.buf_read(owner_proc, owner_source_rptr, blk_size);
            }
            const auto owner_source = owner_source_buf.get();
            
            if (self.is_lazy_merge_enabled() && migration_enabled)
            {
                CMPTH_P_PROF_SCOPE(P, tx_merge_remote_put_1);

                const auto owner_snapshot_rptr = self.get_remote_snapshot_rptr(owner_proc, blk_llk);
                // Write back owner_source_buf (= remote private) to owner_proc.
                rma.write(owner_proc, owner_snapshot_rptr, owner_source_buf.get(), blk_size);
            }
            
            if (is_written) {
                // Use SIMD if the private data is write-protected.
                const bool use_simd = begin_ret.is_write_protected;
                
                // Three copies (local_snapshot, local_working, owner_source) are different with each other.
                // It is necessary to merge them to complete the release.
                merge_policy_type::write_merge(blk_llk, owner_source, local_working, local_snapshot, blk_size, use_simd);
            }
            else {
                CMPTH_P_PROF_SCOPE(P, tx_merge_remote_memcpy);
                
                // Although this process doesn't release this block at this time,
                // the buffer read from the current owner can be utilized.
                // This is important when an acquire on this block is on-going
                // because that thread requires this releaser thread
                // to make the latest modifications visible on this process.
                // Note: The timestamp should also be updated in the directory later.
                std::memcpy(local_working, owner_source, blk_size);
                //std::copy(owner_source, owner_source + blk_size, local_working);
                std::memcpy(local_snapshot, owner_source, blk_size);
                //std::copy(owner_source, owner_source + blk_size, local_snapshot);
            }
        }
        
        if (!migration_enabled && is_written)
        {
            CMPTH_P_PROF_SCOPE(P, tx_merge_remote_put_2);

            // Write back local_snapshot to owner_proc.
            rma.buf_write(owner_proc, owner_source_rptr, local_snapshot, blk_size);
        }
        
        if (begin_ret.needs_protect_after) {
            CMPTH_P_PROF_SCOPE(P, mprotect_tx_after);
            
            // If this block was inaccessible (invalid-clean or invalid-dirty) from the application,
            // make the block readable now.
            
            // Call mprotect(PROT_READ).
            self.protect_readonly(blk_llk);
        }
        
        return { is_written, migration_enabled };
    }
};

} // namespace medsm3
} // namespace menps

