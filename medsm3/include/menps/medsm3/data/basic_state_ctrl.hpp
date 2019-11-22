
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_state_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_base_type = typename P::blk_global_lock_base_type;
    using blk_state_type = typename P::blk_state_type;

    using wr_count_ctrl_type = typename P::wr_count_ctrl_type;
    using wr_count_ctrl_ptr_type = typename P::wr_count_ctrl_ptr_type;

public:
    explicit basic_state_ctrl(wr_count_ctrl_ptr_type wr_count_ctrl_ptr)
        : wr_count_ctrl_ptr_{fdn::move(wr_count_ctrl_ptr)}
    { }

    bool is_invalid(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        const auto state = self.get_local_state(blk_llk);
        return state == blk_state_type::invalid_clean || state == blk_state_type::invalid_dirty;
    }

    struct fast_read_result {
        bool    is_dirty;
    };

    fast_read_result fast_read(blk_local_lock_type& blk_llk)
    {
        CMPTH_P_ASSERT(P, this->is_invalid(blk_llk));
        
        auto& self = this->derived();
        const auto state = self.get_local_state(blk_llk);

        const bool is_dirty = state == blk_state_type::invalid_dirty;

        // Change the state to read-only while preserving the dirty state.
        const auto next_state =
                is_dirty
            ?   blk_state_type::readonly_dirty
            :   blk_state_type::readonly_clean;

        self.set_local_state(blk_llk, next_state);

        return { is_dirty };
    }

    struct start_write_result {
        // Indicate that this block must be changed to PROT_WRITE.
        bool needs_protect;
        // Indicate that this block must be twinned.
        bool needs_twin;
        // Indicate that this block was or is now writable.
        bool is_writable;
    };

    start_write_result start_write(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        const auto state = self.get_local_state(blk_llk);

        if (state == blk_state_type::readonly_clean || state == blk_state_type::readonly_dirty) {
            // Twinning is only required when only the private data is valid.
            const auto needs_twin = state == blk_state_type::readonly_clean;
            
            // Change the state to writable only when it was read-only.
            self.set_local_state(blk_llk, blk_state_type::writable);
            
            // FIXME: is this necessary?
            #if 0
            #ifdef MEDSM2_ENABLE_FAST_RELEASE
            // Update the timestamps locally.
            this->update_ts(rd_ts_st, blk_pos);
            #endif
            #endif
            
            return { true, needs_twin, true };
        }
        else if (state == blk_state_type::writable || state == blk_state_type::pinned) {
            // This block is already writable.
            return { false, false, true };
        }
        else {
            // This block was invalid.
            // TODO: This simply seems an error.
            return { false, false, false };
        }
    }

    struct check_release_result
    {
        // This block is released after this check.
        bool needs_release;
        // This block can use "fast release".
        bool is_fast_released;
    };
    
    check_release_result check_release(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        const auto state = self.get_local_state(blk_llk);
        
        // If this block is clean (invalid_clean or readonly_clean),
        // this block is already downgraded and needs not to be released.
        // If not, this block must be released.
        const auto needs_release =
            ! (state == blk_state_type::invalid_clean ||
               state == blk_state_type::readonly_clean);
        
        return { needs_release, this->wr_count_ctrl().is_fast_released(blk_llk) };
    }

    struct update_global_begin_result
    {
        // Indicates that this block was updated by another process
        // excluding this process.
        bool            is_remotely_updated;
        // Indicates that the block data of this process is in a dirty state.
        // This doesn't exactly correspond to "is_written"
        // because the application may not modify the actual data of the writable pages.
        bool            is_dirty;
        // Indicates that this block must be write-protected (mprotect(PROT_READ))
        // before modifying the private data on this process.
        bool            needs_protect_before;
        // Indicates that this block will become readable (mprotect(PROT_READ))
        // after updating the block in this global lock.
        bool            needs_protect_after;
        // Indicates that this block is write-protected during the merge phase.
        // If needs_protect_before == true, the write protection must be done
        // before modifying the data.
        bool            is_write_protected;
        // Indicates that the local private data will be copied to the public data.
        bool            needs_local_copy;
        // Indicates that the local data pair will be compared.
        bool            needs_local_comp;
    };

    update_global_begin_result update_global_begin(blk_global_lock_base_type& blk_glk, const bool is_remotely_updated_ts)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();
        const auto state = self.get_local_state(blk_llk);

        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();
        const auto owner = blk_glk.prev_owner();

        const auto is_remotely_updated =
                self.is_migration_enabled()
            ?   owner != this_proc
                // TODO: Comparing the write timestamps doesn't work
                //       because write notices may update those values.
                //cur_wr_ts < owner_wr_ts;
            :   (
                    // If invalid, always consider that the remote process has updated this block.
                    (state == blk_state_type::invalid_dirty || state == blk_state_type::invalid_clean)
                    // Compare the write timestamps.
                ||  is_remotely_updated_ts
                );
        
        const auto is_dirty =
            state == blk_state_type::invalid_dirty ||
            state == blk_state_type::readonly_dirty ||
            state == blk_state_type::writable ||
            state == blk_state_type::pinned;
        
        const auto needs_protect_before =
            state == blk_state_type::writable &&
            (is_remotely_updated || this->wr_count_ctrl().needs_protect_before(blk_glk));
        
        const auto needs_protect_after =
            state == blk_state_type::invalid_clean || state == blk_state_type::invalid_dirty;
        
        const bool is_write_protected =
            ! (state == blk_state_type::writable || state == blk_state_type::pinned)
            || needs_protect_before;
        
        const bool needs_local_copy = this->wr_count_ctrl().needs_local_copy(blk_glk);
        
        const bool needs_local_comp = this->wr_count_ctrl().needs_local_comp(blk_glk);

        return { is_remotely_updated, is_dirty, needs_protect_before, needs_protect_after,
            is_write_protected, needs_local_copy, needs_local_comp };
    }

    template <typename MergeResult>
    void update_global_end(blk_global_lock_base_type& blk_glk, const update_global_begin_result& bt_ret, const MergeResult& mg_ret)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();
        
        if (bt_ret.is_write_protected) {
            // (1) If the block was invalid (invalid_clean or invalid_dirty),
            // the latest block was read from the last owner and it becomes readonly_clean.
            //
            // (2) If the block was readonly_dirty, the modification on this block
            // is merged in this process and it becomes readonly_clean.
            //
            // (3) If the block was writable and "becomes clean" (this_proc != old_owner),
            // this block is now downgraded to read-only
            // because the old owner wrote on this block.
            // The state is updated to readonly_clean because of this downgrading.
            
            self.set_local_state(blk_llk, blk_state_type::readonly_clean);
        }
        else {
            // This block was not write-protected in this release
            // and still can be written in this process.
        }

        this->wr_count_ctrl().update_global_end(blk_glk, bt_ret, mg_ret);
    }

    struct invalidate_result {
        // It is necessary to call mprotect(PROT_NONE) on this block.
        bool    needs_protect;
        // This block must be merged from the writer because it is writable.
        bool    needs_merge;
    };
    
    invalidate_result invalidate(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        const auto state = self.get_local_state(blk_llk);
        
        if (state == blk_state_type::invalid_clean || state == blk_state_type::invalid_dirty) {
            // Although the write is not ignored,
            // invalidated blocks require neither protection nor merging.
            return { false, false };
        }
        else if (state == blk_state_type::readonly_clean) {
            // Set the state to "invalid & clean".
            self.set_local_state(blk_llk, blk_state_type::invalid_clean);
            
            // mprotect(PROT_NONE) must be called immediately.
            return { true, false };
        }
        else if (state == blk_state_type::readonly_dirty) {
            // Set the state to "invalid & dirty".
            self.set_local_state(blk_llk, blk_state_type::invalid_dirty);
            
            // mprotect(PROT_NONE) must be called immediately.
            return { true, false };
        }
        else if (state == blk_state_type::writable) {
            // Set the state to "invalid & dirty".
            // The next read will be a merge in this state.
            self.set_local_state(blk_llk, blk_state_type::invalid_dirty);
            
            // mprotect(PROT_NONE) must be called immediately.
            return { true, false };
        }
        else {
            // Pinned blocks cannot be merged with mprotect(PROT_NONE).
            return { false, true };
        }
    }
    
    void set_pinned(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        // Upgrading is necessary before store-releasing.
        CMPTH_P_ASSERT_ALWAYS(P, self.get_local_state(blk_llk) == blk_state_type::writable);
        
        // Set the state to "pinned".
        self.set_local_state(blk_llk, blk_state_type::pinned);
    }
    
    void set_unpinned(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        // Unpinned blocks must be pinned before.
        CMPTH_P_ASSERT_ALWAYS(P, self.get_local_state(blk_llk) == blk_state_type::pinned);
        
        // Set the state to "writable".
        self.set_local_state(blk_llk, blk_state_type::writable);
    }

private:
    wr_count_ctrl_type& wr_count_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->wr_count_ctrl_ptr_);
        return *this->wr_count_ctrl_ptr_;
    }

    wr_count_ctrl_ptr_type wr_count_ctrl_ptr_;
};

} // namespace medsm3
} // namespace menps

