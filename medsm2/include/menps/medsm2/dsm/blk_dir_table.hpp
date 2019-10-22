
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class blk_dir_table
{
    MEFDN_DEFINE_DERIVED(P)
    
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    using sharer_map_type = typename P::sharer_map_type;
    #else
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    #endif
    
    using rd_set_type = typename P::rd_set_type;
    using wr_set_type = typename P::wr_set_type;
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    using wr_count_type = typename P::wr_count_type;

    using ult_itf_type      = typename P::ult_itf_type;
    
    enum class state_type {
        invalid_clean = 0
    ,   invalid_dirty
    ,   readonly_clean
    ,   readonly_dirty
    ,   writable
    ,   pinned   // special state for call stacks
    };
    
public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        auto& com = conf.com;
        auto& coll = com.get_coll();
        const auto this_proc = coll.this_proc_id();
        
        this->num_blks_ = conf.num_blks; // for debugging
        
        // Initialize local entries with zero.
        this->les_ = mefdn::make_unique<local_entry []>(conf.num_blks);
        
        for (blk_pos_type blk_pos = 0; blk_pos < conf.num_blks; ++blk_pos) {
            // Set this process as the "first home".
            // This means that every reader starts with its data first.
            // TODO: This is a bit weird
            //       because "home" is not the current process in general.
            this->les_[blk_pos].home_proc = this_proc;
            
            #ifdef MEDSM2_ENABLE_FAST_RELEASE
            this->les_[blk_pos].fast_rel_threshold = 1;
            #endif
        }

        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        auto& rma = com.get_rma();
        const auto num_procs = com.get_num_procs();
        const auto sharer_map_size = sharer_map_type::get_sharer_map_size(num_procs);
        this->inv_flags_.coll_make(rma, coll, conf.num_blks * sharer_map_size);
        #endif
    }
    
    unique_lock_type get_local_lock(const blk_pos_type blk_pos)
    {
        MEFDN_ASSERT(blk_pos < num_blks_);
        
        MEFDN_LOG_VERBOSE(
            "msg:Locking dir lock.\t"
            "blk_pos:{}"
        ,   blk_pos
        );
        
        auto& le = this->les_[blk_pos];
        return unique_lock_type(le.mtx);
    }
    
    struct start_read_result
    {
        // Indicate that this block must be fetched from the home process.
        bool needs_read;
        // Indicate that this process needs to read the latest block from the current owner
        // because the read timestamp is older than the required value.
        bool needs_latest_read;
        // Indicate that this block was modified by this process.
        bool is_dirty;
        // The home process indicates the source of reading.
        proc_id_type home_proc;
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        // Self-invalidation timestamp means when this block must be invalidated.
        rd_ts_type rd_ts;
        #endif
    };
    
    MEFDN_NODISCARD
    start_read_result start_read(
        rd_set_type&            rd_set
    ,   const blk_id_type       blk_id
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        
        if (!(le.state == state_type::invalid_clean || le.state == state_type::invalid_dirty)) {
            // This block is not marked as invalid.
            return { false, false, false
                #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
                , 0, 0
                #endif
                };
        }
        else {
            #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
            // Load the current read timestamp.
            const auto cur_rd_ts = le.cur_rd_ts;
            
            const auto rd_ts_st = rd_set.get_ts_state();
            #endif
            
            // If the timestamp is older than the minimum read timestamp,
            // this block must be loaded from the latest owner
            // because it was self-invalidated via timestamps
            // and this process doesn't know a valid sharer for it.
            //
            // If not, the home process written in this process is still valid.
            // (= before self-invalidation.)
            const auto needs_latest_read =
                #if defined(MEDSM2_USE_DIRECTORY_COHERENCE)
                true
                #else
                ! rd_ts_st.is_valid_rd_ts(cur_rd_ts)
                #endif
                ;
            
            const auto is_dirty =
                le.state == state_type::invalid_dirty;
            
            if (needs_latest_read) {
                // Later, the caller updates this block inside the global critical section.
                // The block state and existence in the read set are handled in unlock_global().
            }
            else {
                // Change the state to read-only only when it was invalid.
                le.state = is_dirty ? state_type::readonly_dirty : state_type::readonly_clean;
                
                // Add this block to the read set.
                #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
                rd_set.add_readable(blk_id);
                #else
                rd_set.add_readable(blk_id, cur_rd_ts);
                #endif
            }
            
            return { true, needs_latest_read, is_dirty, le.home_proc
                #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
                , cur_rd_ts
                #endif
                };
        }
    }
    
    struct start_write_result
    {
        // Indicate that this block must be changed to PROT_WRITE.
        bool needs_protect;
        // Indicate that this block must be twinned.
        bool needs_twin;
        // Indicate that this block was or is now writable.
        bool is_writable;
    };
    
    MEFDN_NODISCARD
    start_write_result start_write(
        wr_set_type&            wr_set
    ,   const rd_ts_state_type& rd_ts_st
    ,   const blk_id_type       blk_id
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        if (le.state == state_type::readonly_clean || le.state == state_type::readonly_dirty) {
            // Twinning is only required when only the private data is valid.
            const auto needs_twin =
                le.state == state_type::readonly_clean;
            
            // Change the state to writable only when it was read-only.
            le.state = state_type::writable;
            
            // Add this block to the write set.
            wr_set.add_writable(blk_id);
            
            #ifdef MEDSM2_ENABLE_FAST_RELEASE
            // Update the timestamps locally.
            this->update_ts(rd_ts_st, blk_pos);
            #endif
            
            return { true, needs_twin, true };
        }
        else if (le.state == state_type::writable || le.state == state_type::pinned)
        {
            // This block is already writable.
            return { false, false, true };
        }
        else {
            // This block was invalid.
            // TODO: This simply seems an error.
            return { false, false, false };
        }
    }
    
    void set_pinned(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        // Upgrading is necessary before store-releasing.
        MEFDN_ASSERT(le.state == state_type::writable);
        
        // Set the state to "pinned".
        le.state = state_type::pinned;
    }
    
    void set_unpinned(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        // Unpinned blocks must be pinned before.
        MEFDN_ASSERT(le.state == state_type::pinned);
        
        // Set the state to "writable".
        le.state = state_type::writable;
    }
    
    struct check_release_result
    {
        // This block is released after this check.
        bool needs_release;
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        // This block can use "fast release".
        bool is_fast_released;
        #endif
    };
    
    check_release_result check_release(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        const auto& le = this->les_[blk_pos];
        const auto state = le.state;
        
        // If this block is clean (invalid_clean or readonly_clean),
        // this block is already downgraded and needs not to be released.
        // If not, this block must be released.
        const auto needs_release =
            ! (state == state_type::invalid_clean ||
               state == state_type::readonly_clean);
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        const auto is_fast_released =
            le.fast_rel_count < le.fast_rel_threshold;
        
        return { needs_release, is_fast_released };
        #else
        return { needs_release };
        #endif
    }
    
    #ifdef MEDSM2_ENABLE_FAST_RELEASE
    struct fast_release_result
    {
        wr_ts_type  new_wr_ts;
        rd_ts_type  new_rd_ts;
    };
    
    fast_release_result fast_release(
        const rd_ts_state_type& rd_ts_st
    ,   const blk_id_type       blk_id MEFDN_MAYBE_UNUSED
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& self = this->derived();
        auto& le = this->les_[blk_pos];
        
        const auto ge = self.read_lock_entry(blk_pos);
        
        // Update the timestamps because it's possible to consider that
        // no other processes have updated them.
        le.cur_wr_ts = ge.home_wr_ts;
        le.cur_rd_ts = ge.home_rd_ts;
        
        const auto new_ts =
            this->update_ts(rd_ts_st, blk_pos);
        
        ++le.fast_rel_count;
        
        MEFDN_LOG_VERBOSE(
            "msg:Fast release.\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "cur_wr_ts:{}\t"
            "cur_rd_ts:{}\t"
            "new_wr_ts:{}\t"
            "new_rd_ts:{}\t"
            "fast_rel_count:{}"
        ,   blk_id
        ,   blk_pos
        ,   le.cur_wr_ts
        ,   le.cur_rd_ts
        ,   new_ts.wr_ts
        ,   new_ts.rd_ts
        ,   le.fast_rel_count
        );
        
        return { new_ts.wr_ts, new_ts.rd_ts };
    }
    #endif
    
private:
    struct invalidate_result {
        bool    is_ignored;
        // It is necessary to call mprotect(PROT_NONE) on this block.
        bool    needs_protect;
        // This block must be merged from the writer because it is writable.
        bool    needs_merge;
    };
    
    invalidate_result set_invalid_state(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const bool              is_invalidated
    ) {
        auto& le = this->les_[blk_pos];
        
        if (is_invalidated) {
            if (le.state == state_type::invalid_clean || le.state == state_type::invalid_dirty) {
                // Although the write is not ignored,
                // invalidated blocks require neither protection nor merging.
                return { false, false, false };
            }
            else if (le.state == state_type::readonly_clean) {
                // Set the state to "invalid & clean".
                le.state = state_type::invalid_clean;
                
                // mprotect(PROT_NONE) must be called immediately.
                return { false, true, false };
            }
            else if (le.state == state_type::readonly_dirty) {
                // Set the state to "invalid & dirty".
                le.state = state_type::invalid_dirty;
                
                // mprotect(PROT_NONE) must be called immediately.
                return { false, true, false };
            }
            else if (le.state == state_type::writable) {
                // Set the state to "invalid & dirty".
                // The next read will be a merge in this state.
                le.state = state_type::invalid_dirty;
                
                // mprotect(PROT_NONE) must be called immediately.
                return { false, true, false };
            }
            else {
                // Pinned blocks cannot be merged with mprotect(PROT_NONE).
                return { false, false, true };
            }
        }
        else {
            // The write notice is ignored because it's old.
            // There is no need to release/protect this block.
            return { true, false, false };
        }
    }
    
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
public:
    using self_invalidate_result = invalidate_result;
    
    self_invalidate_result self_invalidate(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        const bool is_invalidated = *inv_flags_.local(blk_pos);
        return this->set_invalid_state(blk_pos, lk, is_invalidated);
    }
    
    #else // MEDSM2_USE_DIRECTORY_COHERENCE
private:
    invalidate_result invalidate(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const wr_ts_type        new_wr_ts
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        
        const auto rd_ts = le.cur_rd_ts;
        
        // Check whether the timestamp is newer or not.
        // If old_rd_ts < new_wr_ts, the read timestamp has expired
        // and this block was invalidated.
        const auto is_invalidated = P::is_greater_rd_ts(new_wr_ts, rd_ts);
        return this->set_invalid_state(blk_pos, lk, is_invalidated);
    }
    
public:
    using acquire_result = invalidate_result;
    
    // Note that the home process and the block timestamp
    // may have changed regardless of the returned values.
    MEFDN_NODISCARD
    acquire_result acquire(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const proc_id_type      new_home_proc
    ,   const rd_ts_type        new_rd_ts
    ,   const wr_ts_type        new_wr_ts
    ) {
        const auto ret = this->invalidate(blk_pos, lk, new_wr_ts);
        
        if (! ret.is_ignored) {
            // Even if the block is already invalid,
            // the fields for this block must be updated
            // based on this write notice to prepare for the next read.
            
            auto& le = this->les_[blk_pos];
            
            // Update the home process and timestamps.
            le.home_proc = new_home_proc;
            le.cur_wr_ts = new_wr_ts;
            le.cur_rd_ts = new_rd_ts;
        }
        
        return ret;
    }
    
    struct self_invalidate_result {
        bool        is_ignored;
        bool        needs_protect;
        bool        needs_merge;
        wr_ts_type  wr_ts;
        rd_ts_type  rd_ts;
    };
    
    self_invalidate_result self_invalidate(
        const rd_ts_state_type& rd_ts_st
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        const auto ret = this->invalidate(blk_pos, lk, rd_ts_st.get_min_wr_ts());
        
        const auto& le = this->les_[blk_pos];
        
        // Load the current timestamps.
        const auto wr_ts = le.cur_wr_ts;
        const auto rd_ts = le.cur_rd_ts;
        
        return { ret.is_ignored, ret.needs_protect, ret.needs_merge, wr_ts, rd_ts };
    }
    #endif // MEDSM2_USE_DIRECTORY_COHERENCE
    
    struct begin_transaction_result {
        // The latest owner's ID.
        proc_id_type    owner;
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        // The write timestamp read from the latest owner.
        wr_ts_type      wr_ts;
        // The read timestamp read from the latest owner.
        rd_ts_type      rd_ts;
        #endif
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
    
    template <typename LockResult>
    begin_transaction_result begin_transaction(
        com_itf_type&           com
    ,   const blk_id_type       blk_id MEFDN_MAYBE_UNUSED
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const LockResult&       glk_ret
    ) {
        this->check_locked(blk_pos, lk);
        CMPTH_P_PROF_SCOPE(P, tx_begin);
        
        const auto this_proc = com.this_proc_id();
        
        const auto owner = glk_ret.owner;
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        const auto owner_wr_ts = glk_ret.home_wr_ts;
        const auto owner_rd_ts = glk_ret.home_rd_ts;
        #endif
        
        auto& le = this->les_[blk_pos];
        
        const auto state = le.state;
        const auto wr_count = le.wr_count;
        
        const auto is_remotely_updated =
            #ifdef MEDSM2_ENABLE_MIGRATION
            owner != this_proc;
            // TODO: Comparing the write timestamps doesn't work
            //       because write notices may update those values.
            //cur_wr_ts < owner_wr_ts;
            #else
            // If invalid, always consider that the remote process has updated this block.
            (state == state_type::invalid_dirty || state == state_type::invalid_clean) ||
            // Compare the write timestamps.
            le.cur_wr_ts < owner_wr_ts;
            #endif
        
        const auto is_dirty =
            state == state_type::invalid_dirty ||
            state == state_type::readonly_dirty ||
            state == state_type::writable ||
            state == state_type::pinned;
        
        const auto needs_protect_before =
            state == state_type::writable &&
            (is_remotely_updated || wr_count >= P::wr_count_threshold);
        
        const auto needs_protect_after =
            state == state_type::invalid_clean || state == state_type::invalid_dirty;
        
        const bool is_write_protected =
            ! (state == state_type::writable || state == state_type::pinned)
            || needs_protect_before;
        
        const bool needs_local_copy =
        #ifdef MEDSM2_ENABLE_LAZY_MERGE
            (wr_count+2) >= P::wr_count_threshold;
        #else
            true;
        #endif
        
        const bool needs_local_comp =
        #ifdef MEDSM2_ENABLE_NEEDS_LOCAL_COMP
            (wr_count+1) >= P::wr_count_threshold;
        #else
            true;
        #endif
        
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        MEFDN_LOG_VERBOSE(
            "msg:Begin transaction for cache block.\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "owner:{}\t"
            "is_remotely_updated:{}\t"
            "is_dirty:{}\t"
            "needs_protect_before:{}\t"
            "needs_protect_after:{}\t"
            "is_write_protected:{}\t"
            "needs_local_copy:{}\t"
            "needs_local_comp:{}\t"
        ,   blk_id
        ,   blk_pos
        ,   owner
        ,   is_remotely_updated
        ,   is_dirty
        ,   needs_protect_before
        ,   needs_protect_after
        ,   is_write_protected
        ,   needs_local_copy
        ,   needs_local_comp
        );
        #else
        MEFDN_LOG_VERBOSE(
            "msg:Begin transaction for cache block.\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "owner:{}\t"
            "cur_wr_ts:{}\t"
            "cur_rd_ts:{}\t"
            "owner_wr_ts:{}\t"
            "owner_rd_ts:{}\t"
            "is_remotely_updated:{}\t"
            "is_dirty:{}\t"
            "needs_protect_before:{}\t"
            "needs_protect_after:{}\t"
            "is_write_protected:{}\t"
            "needs_local_copy:{}\t"
            "needs_local_comp:{}\t"
        ,   blk_id
        ,   blk_pos
        ,   owner
        ,   le.cur_wr_ts
        ,   le.cur_rd_ts
        ,   owner_wr_ts
        ,   owner_rd_ts
        ,   is_remotely_updated
        ,   is_dirty
        ,   needs_protect_before
        ,   needs_protect_after
        ,   is_write_protected
        ,   needs_local_copy
        ,   needs_local_comp
        );
        #endif
        
        return { owner,
            #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
            owner_wr_ts, owner_rd_ts,
            #endif
            is_remotely_updated, is_dirty,
            needs_protect_before, needs_protect_after, is_write_protected,
            needs_local_copy, needs_local_comp };
    }
    
    struct end_transaction_result {
        proc_id_type    new_owner;
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        sharer_map_type sharers;
        #else
        rd_ts_type      new_rd_ts;
        wr_ts_type      new_wr_ts;
        #endif
        // Indicate that this block must not be removed from the write set
        // because the pages are still (PROT_READ|PROT_WRITE).
        bool            is_still_writable;
    };
    
    template <typename LockResult, typename MergeResult>
    end_transaction_result end_transaction(
        com_itf_type&                   com
    ,   const rd_ts_state_type&         rd_ts_st
    ,   const blk_id_type               blk_id
    ,   const blk_pos_type              blk_pos
    ,   const unique_lock_type&         lk
    ,   LockResult&                     glk_ret MEFDN_MAYBE_UNUSED
    ,   const begin_transaction_result& bt_ret
    ,   const MergeResult&              mg_ret
    ) {
        this->check_locked(blk_pos, lk);
        CMPTH_P_PROF_SCOPE(P, tx_end);
        
        const auto this_proc = com.this_proc_id();
        
        const auto is_migrated = mg_ret.is_migrated;
        // Note: new_owner could be the same as old_owner in the past.
        const auto new_owner = is_migrated ? this_proc : bt_ret.owner;
        
        auto& le = this->les_[blk_pos];
        
        // Update the home process.
        // The home process is only updated locally.
        le.home_proc = new_owner;
        
        const auto old_state = le.state;
        auto new_state = old_state;
        
        auto my_rd_ts_st = rd_ts_st;
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        if (old_state == state_type::invalid_dirty || old_state == state_type::invalid_clean) {
            auto& rd_set = my_rd_ts_st.get_rd_set();
            
            // Reload the timestamp state.
            my_rd_ts_st = rd_set.get_ts_state();
        }
        
        // Generate new timestamp values.
        const auto new_ts =
            this->make_new_ts(
                my_rd_ts_st, mg_ret.is_written, bt_ret.wr_ts, bt_ret.rd_ts
            );
        #endif
        
        if (old_state == state_type::invalid_dirty || old_state == state_type::invalid_clean) {
            auto& rd_set = my_rd_ts_st.get_rd_set();
            
            // This block was invalid and became readable in this transaction.
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            rd_set.add_readable(blk_id);
            #else
            rd_set.add_readable(blk_id, new_ts.rd_ts);
            #endif
        }
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        // Update the timestamps because this process lastly released.
        le.cur_wr_ts = new_ts.wr_ts;
        le.cur_rd_ts = new_ts.rd_ts;
        #endif
        
        const auto is_still_writable = ! bt_ret.is_write_protected;
        
        if (is_still_writable) {
            // This block was not write-protected in this release
            // and still can be written in this process.
        }
        else {
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
            new_state = state_type::readonly_clean;
        }
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        if (bt_ret.is_write_protected) {
            // Reset the counts.
            le.wr_count = 0;
            le.fast_rel_threshold = 1;
        }
        else if (bt_ret.needs_local_comp && mg_ret.is_written) {
            // The block is written.
            le.wr_count = 0;
            le.fast_rel_threshold =
                std::min(
                    le.fast_rel_threshold * 2
                ,   P::max_fast_rel_threshold
                );
        }
        else {
            // The block is not modified.
            le.wr_count += le.fast_rel_count + 1;
            le.fast_rel_threshold = 1;
        }
        le.fast_rel_count = 0;
        // TODO: May overflow.
        #else
        // Update the write count.
        le.wr_count =
            (bt_ret.is_write_protected ||
                (bt_ret.needs_local_comp && mg_ret.is_written))
            ? 0 : (le.wr_count + 1);
        // TODO: May overflow.
        #endif
        
        le.state = new_state;

        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        auto sharers = mefdn::move(glk_ret.sharers);
        if (mg_ret.is_written) {
            // Send invalidation messages.
            auto& rma = com.get_rma();
            const auto num_procs = com.get_num_procs();
            std::vector<proc_id_type> inv_sharers;
            for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
                if (proc_id != this_proc && sharers.is_set(proc_id)) {
                    inv_sharers.push_back(proc_id);
                    sharers.unset(proc_id);
                }
            }
            const bool written_flag = true;
            ult_itf_type::for_loop(
                ult_itf_type::execution::par
            ,   0, inv_sharers.size()
            ,   [&] (const mefdn::size_t i) {
                    const auto proc_id = inv_sharers[i];
                    rma.buf_write(
                        proc_id
                    ,   this->inv_flags_.remote(proc_id, blk_pos)
                    ,   &written_flag
                    ,   1
                    );
                }
            );
        }
        
        // Set this process as a sharer.
        sharers.set(this_proc);
        
        // Reset the invalidation flag for this process.
        *this->inv_flags_.local(blk_pos) = false;
        #endif
        
        return { new_owner,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            mefdn::move(sharers),
            #else
            new_ts.rd_ts, new_ts.wr_ts,
            #endif
            is_still_writable };
    }
    
    #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
private:
    struct make_new_ts_result {
        wr_ts_type  wr_ts;
        rd_ts_type  rd_ts;
    };
    
    make_new_ts_result make_new_ts(
        const rd_ts_state_type& rd_ts_st
    ,   const bool              is_written
    ,   const wr_ts_type        wr_ts
    ,   const rd_ts_type        rd_ts
    ) {
        // If the data is written, the timestamp is updated.
        // make_new_wr_ts() calculates new_wr_ts = max(old_rd_ts+1, acq_ts).
        // If not, because the data is read from the old owner
        // (that will be the owner again),
        // the timestamp becomes equal to that of the owner.
        const auto new_wr_ts =
            is_written ? rd_ts_st.make_new_wr_ts(rd_ts) : wr_ts;
        
        // The read timestamp depends on the write timestamp.
        // This calculates new_rd_ts = max(new_wr_ts+lease, old_rd_ts).
        // TODO: Provide a good prediction for a lease value of each block.
        const auto new_rd_ts =
            rd_ts_st.make_new_rd_ts(new_wr_ts, rd_ts);
        
        return { new_wr_ts, new_rd_ts };
    }
    
    #ifdef MEDSM2_ENABLE_FAST_RELEASE
    make_new_ts_result update_ts(
        const rd_ts_state_type& rd_ts_st
    ,   const blk_pos_type      blk_pos
    ) {
        auto& self = this->derived();
        const auto ge = self.read_lock_entry(blk_pos);
        
        const auto old_wr_ts = ge.home_wr_ts;
        const auto old_rd_ts = ge.home_rd_ts;
        
        const auto new_ts =
            this->make_new_ts(rd_ts_st, true, old_wr_ts, old_rd_ts);
        
        self.write_lock_entry(blk_pos, new_ts.wr_ts, new_ts.rd_ts);
        
        // Return the "old" timestamps
        // which can be used for write notices.
        return { old_wr_ts, old_rd_ts };
    }
    #endif
    #endif // MEDSM2_USE_DIRECTORY_COHERENCE
    
public:
    void check_locked(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        MEFDN_ASSERT(lk.mutex() == &this->les_[blk_pos].mtx);
        MEFDN_ASSERT(lk.owns_lock());
    }
    
private:
    struct local_entry {
        mutex_type      mtx;
        // A home is a process from which the reader must read
        // the block data to maintain coherence.
        // This field is only updated via write notices
        // and remote writers cannot access it.
        proc_id_type    home_proc;
        // The block state indicates the page protection
        // of the local data block.
        state_type      state;
        // The number of transactions without writing the data.
        wr_count_type   wr_count;
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        wr_ts_type      cur_wr_ts;
        rd_ts_type      cur_rd_ts;
        #endif
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        wr_count_type   fast_rel_count;
        
        wr_count_type   fast_rel_threshold;
        #endif
    };
    
    mefdn::size_t num_blks_ = 0;
    
    mefdn::unique_ptr<local_entry []> les_;

    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    typename P::template alltoall_buffer<bool> inv_flags_;
    #endif
};

} // namespace medsm2
} // namespace menps

