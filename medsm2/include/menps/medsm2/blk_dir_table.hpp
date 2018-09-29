
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <stdexcept>

//#define MEDSM2_USE_ONE_MUTEX
//#define MEDSM2_FORCE_LATEST_READ

namespace menps {
namespace medsm2 {

template <typename P>
class blk_dir_table
{
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
    using rd_set_type = typename P::rd_set_type;
    using wr_set_type = typename P::wr_set_type;
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    using wr_count_type = typename P::wr_count_type;
    
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
        auto& coll = conf.com.get_coll();
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
        }
        
        this->ges_.coll_make(conf.com.get_rma(), coll, conf.num_blks);
    }
    
    unique_lock_type get_local_lock(const blk_pos_type blk_pos)
    {
        MEFDN_ASSERT(blk_pos < num_blks_);
        
        MEFDN_LOG_VERBOSE(
            "msg:Locking dir lock.\t"
            "blk_pos:{}"
        ,   blk_pos
        );
        
        #ifdef MEDSM2_USE_ONE_MUTEX
        return unique_lock_type(this->mtx_);
        #else
        auto& le = this->les_[blk_pos];
        return unique_lock_type(le.mtx);
        #endif
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
        // Self-invalidation timestamp means when this block must be invalidated.
        rd_ts_type rd_ts;
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
        auto& ge = * this->ges_.local(blk_pos);
        
        if (!(le.state == state_type::invalid_clean || le.state == state_type::invalid_dirty)) {
            // This block is not marked as invalid.
            return { false, false, false, 0, 0 };
        }
        else {
            // Load the current read timestamp.
            // This variable is only updated when the local directory lock is acquired.
            const auto cur_rd_ts = ge.home_rd_ts;
            
            const auto rd_ts_st = rd_set.get_ts_state();
            
            // If the timestamp is older than the minimum read timestamp,
            // this block must be loaded from the latest owner
            // because it was self-invalidated via timestamps
            // and this process doesn't know a valid sharer for it.
            //
            // If not, the home process written in this process is still valid.
            // (= before self-invalidation.)
            const auto needs_latest_read =
                #ifdef MEDSM2_FORCE_LATEST_READ
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
                rd_set.add_readable(blk_id, cur_rd_ts);
            }
            
            return { true, needs_latest_read, is_dirty, le.home_proc, cur_rd_ts };
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
        
        return { needs_release };
    }
    
    #ifdef MEDSM2_ENABLE_FAST_RELEASE
    
    struct fast_release_result
    {
        wr_ts_type  new_wr_ts;
        rd_ts_type  new_rd_ts;
    };
    
    fast_release_result fast_release(
        const rd_ts_state_type& rd_ts_st
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        const auto new_ts =
            this->update_ts(rd_ts_st, blk_pos);
        
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
    
    invalidate_result invalidate(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const wr_ts_type        new_wr_ts
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        auto& ge = * this->ges_.local(blk_pos);
        
        const auto rd_ts = ge.home_rd_ts;
        
        // Check whether the timestamp is newer or not.
        // If old_rd_ts < new_wr_ts, the read timestamp has expired
        // and this block was invalidated.
        if (P::is_greater_rd_ts(new_wr_ts, rd_ts)) {
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
            auto& ge = * this->ges_.local(blk_pos);
            
            // Update the home process.
            le.home_proc = new_home_proc;
            // Update the read timestamp.
            // Note: It is not trivial that these timestamp variables
            //       can be used for storing the timestamps of "another writer process".
            //       This is safe because this process is not the home process
            //       if another writer writes on this block.
            ge.home_rd_ts = new_rd_ts;
            
            // Update the write timestamp.
            ge.home_wr_ts = new_wr_ts;
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
        
        auto& ge = * this->ges_.local(blk_pos);
        const auto wr_ts = ge.home_wr_ts;
        const auto rd_ts = ge.home_rd_ts;
        
        return { ret.is_ignored, ret.needs_protect, ret.needs_merge, wr_ts, rd_ts };
    }
    
    struct begin_transaction_result {
        // The latest owner's ID.
        proc_id_type    owner;
        // The write timestamp read from the latest owner.
        wr_ts_type      wr_ts;
        // The read timestamp read from the latest owner.
        rd_ts_type      rd_ts;
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
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const LockResult&       glk_ret
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        const auto owner = glk_ret.owner;
        
        const auto ge_rptr = this->ges_.remote(owner, blk_pos);
        
        // Load the latest timestamps.
        const auto owner_ge_buf =
            rma.buf_read(owner, ge_rptr, 1);
        // TODO: It's better if this read is overlapped with other communications.
        
        const auto owner_ge = *owner_ge_buf.get();
        
        const auto owner_wr_ts = owner_ge.home_wr_ts;
        const auto owner_rd_ts = owner_ge.home_rd_ts;
        
        auto& le = this->les_[blk_pos];
        auto& ge MEFDN_MAYBE_UNUSED = * this->ges_.local(blk_pos);
        
        const auto state = le.state;
        //const auto cur_wr_ts = ge.wr_ts;
        const auto wr_count = le.wr_count;
        
        const auto is_remotely_updated =
            owner != this_proc;
            // TODO: Comparing the write timestamps doesn't work
            //       because write notices may update those values.
            //cur_wr_ts < owner_wr_ts;
        
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
        
        MEFDN_LOG_VERBOSE(
            "msg:Locked global lock.\t"
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
        ,   blk_pos
        ,   owner
        ,   ge.home_wr_ts
        ,   ge.home_rd_ts
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
        
        return { owner, owner_wr_ts, owner_rd_ts,
            is_remotely_updated, is_dirty,
            needs_protect_before, needs_protect_after, is_write_protected,
            needs_local_copy, needs_local_comp };
    }
    
    struct end_transaction_result {
        proc_id_type    new_owner;
        rd_ts_type      new_rd_ts;
        wr_ts_type      new_wr_ts;
        // Indicate that this block must not be removed from the write set
        // because the pages are still (PROT_READ|PROT_WRITE).
        bool            is_still_writable;
    };
    
    template <typename MergeResult>
    end_transaction_result end_transaction(
        com_itf_type&                   com
    ,   const rd_ts_state_type&         rd_ts_st
    ,   const blk_id_type               blk_id
    ,   const blk_pos_type              blk_pos
    ,   const unique_lock_type&         lk
    ,   const begin_transaction_result& bt_ret
    ,   const MergeResult&              mg_ret
    ) {
        this->check_locked(blk_pos, lk);
        
        #ifndef MEDSM2_ENABLE_FAST_RELEASE
        auto& rma = com.get_rma();
        #endif
        
        const auto cur_proc = com.this_proc_id();
        #ifndef MEDSM2_ENABLE_FAST_RELEASE
        const auto old_owner = bt_ret.owner;
        #endif
        
        // Generate new timestamp values.
        const auto new_ts =
            this->make_new_ts(
                rd_ts_st, mg_ret.is_written, bt_ret.wr_ts, bt_ret.rd_ts
            );
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        const auto new_owner = cur_proc;
        #else
        const auto new_owner = mg_ret.is_written ? cur_proc : old_owner;
        #endif
        
        auto& le = this->les_[blk_pos];
        auto& ge = * this->ges_.local(blk_pos);
        
        // Update the home process.
        // The home process is only updated locally.
        le.home_proc = new_owner;
        
        // Update the timestamps.
        // Although this value may be read by another writer,
        // this process still has the lock for it.
        // Also, the timestamps only increases monotonically.
        ge.home_wr_ts = new_ts.wr_ts;
        ge.home_rd_ts = new_ts.rd_ts;
        
        #ifndef MEDSM2_ENABLE_FAST_RELEASE
        if (new_owner != cur_proc) {
            // This process is not the new owner.
            // (2) old_owner == new_owner != cur_proc
            
            MEFDN_ASSERT(bt_ret.wr_ts == new_ts.wr_ts);
            
            // If the read timestamp is updated in this transaction,
            // it needs to be written to the owner.
            if (bt_ret.rd_ts < new_ts.rd_ts) {
                const auto ge_rptr =
                    this->ges_.remote(new_owner /* == old_owner */, blk_pos);
                
                // Write the new read timestamp to the owner.
                rma.buf_write(
                    new_owner /* == old_owner */
                ,   rma_itf_type::member(ge_rptr, &global_entry::home_rd_ts)
                ,   &new_ts.rd_ts
                ,   1
                );
            }
        }
        #endif
        
        const auto old_state = le.state;
        auto new_state = old_state;
        
        if (old_state == state_type::invalid_dirty || old_state == state_type::invalid_clean) {
            auto& rd_set = rd_ts_st.get_rd_set();
            
            // This block was invalid and became readable in this transaction.
            rd_set.add_readable(blk_id, new_ts.rd_ts);
        }
        
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
            // (3) If the block was writable and "becomes clean" (cur_proc != old_owner),
            // this block is now downgraded to read-only
            // because the old owner wrote on this block.
            // The state is updated to readonly_clean because of this downgrading.
            new_state = state_type::readonly_clean;
        }
        
        // Update the write count.
        le.wr_count =
            (bt_ret.is_write_protected ||
                (bt_ret.needs_local_comp && mg_ret.is_written))
            ? 0 : (le.wr_count + 1);
        // TODO: May overflow.
        
        le.state = new_state;
        
        return { new_owner, new_ts.rd_ts, new_ts.wr_ts, is_still_writable };
    }
    
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
    
    make_new_ts_result update_ts(
        const rd_ts_state_type& rd_ts_st
    ,   const blk_pos_type      blk_pos
    ) {
        auto& ge = * this->ges_.local(blk_pos);
        
        const auto old_wr_ts = ge.home_wr_ts;
        const auto old_rd_ts = ge.home_rd_ts;
        
        const auto new_ts =
            this->make_new_ts(rd_ts_st, true, old_wr_ts, old_rd_ts);
        
        ge.home_wr_ts = new_ts.wr_ts;
        ge.home_rd_ts = new_ts.rd_ts;
        
        // Return the "old" timestamps
        // which can be used for write notices.
        return { old_wr_ts, old_rd_ts };
    }
    
public:
    void check_locked(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        #ifdef MEDSM2_USE_ONE_MUTEX
        MEFDN_ASSERT(lk.mutex() == &this->mtx_);
        #else
        MEFDN_ASSERT(lk.mutex() == &this->les_[blk_pos].mtx);
        #endif
        MEFDN_ASSERT(lk.owns_lock());
    }
    
private:
    struct global_entry {
        // The timestamp when this block must be self-invalidated.
        // If this process is the owner of this block,
        // this member may be modified by other reader processes.
        rd_ts_type  home_rd_ts;
        // The timestamp that can be read by other writers.
        // Only modified by the local process.
        wr_ts_type  home_wr_ts;
        
        rd_ts_type  wn_rd_ts;
        wr_ts_type  wn_wr_ts;
    };
    
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
    };
    
    mefdn::size_t num_blks_ = 0;
    
    mefdn::unique_ptr<local_entry []> les_;
    
    typename P::template
        alltoall_buffer<global_entry> ges_;
    
    #ifdef MEDSM2_USE_ONE_MUTEX
    mutex_type mtx_;
    #endif
};

} // namespace medsm2
} // namespace menps

