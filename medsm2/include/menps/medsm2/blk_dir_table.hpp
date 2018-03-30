
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <stdexcept>

//#define MEDSM2_USE_ONE_MUTEX
//#define MEDSM2_FORCE_LATEST_READ
//#define MEDSM2_CHECK_VALID_LOCKED_LINKS

namespace menps {
namespace medsm2 {

template <typename P>
class blk_dir_table
{
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    using blk_pos_type = typename P::blk_pos_type;
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
    using acq_sig_type = typename P::acq_sig_type;
    
    enum class state_type {
        invalid_clean = 0
    ,   invalid_dirty
    ,   readonly_clean
    ,   readonly_dirty
    ,   writable
    ,   released // special state for store(release)
    ,   pinned   // special state for call stacks
    };
    
public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        this->num_blks_ = conf.num_blks; // for debugging
        
        // Initialize local entries with zero.
        this->les_ = mefdn::make_unique<local_entry []>(conf.num_blks);
        
        auto& coll = conf.com.get_coll();
        
        this->ges_.coll_make(conf.com.get_rma(), coll, conf.num_blks);
        
        const auto this_proc = coll.this_proc_id();
        const auto num_procs = coll.get_num_procs();
        
        for (blk_pos_type blk_pos = 0; blk_pos < this->num_blks_; ++blk_pos) {
            auto& ge = * this->ges_.local(blk_pos);
            
            // The owner of blk_pos is (blk_pos % num_procs).
            const proc_id_type owner = blk_pos % num_procs;
            
            const auto lock_val = 
                    owner == this_proc
                ?   this->make_owned_lock_val(owner)
                :   this->make_linked_lock_val(owner);
            
            ge.lock = lock_val;
        }
        
        #if 0
        // for debugging...
        for (mefdn::size_t i = 0; i < this->num_blks_; ++i) {
            //auto lk = get_local_lock(i);
            //*this->les_[i].mtx.native_handle() = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
            *this->les_[i].mtx.native_handle() = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
        }
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
        #ifdef MEDSM2_RELEASE_LATEST_READ
        // Indicate that this process needs to read the latest block from the current owner
        // because the read timestamp is older than the required value.
        bool needs_latest_read;
        #endif
        // Indicate that this block was modified by this process.
        bool is_dirty;
        // The home process indicates the source of reading.
        proc_id_type home_proc;
        // Self-invalidation timestamp means when this block must be invalidated.
        rd_ts_type rd_ts;
    };
    
    MEFDN_NODISCARD
    start_read_result start_read(
        com_itf_type&           com
    ,   const acq_sig_type&     acq_sig
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        auto& ge = * this->ges_.local(blk_pos);
        
        if (!(le.state == state_type::invalid_clean || le.state == state_type::invalid_dirty)) {
            // This block is not marked as invalid.
            #ifdef MEDSM2_RELEASE_LATEST_READ
            return { false, false, false, 0, 0 };
            #else
            return { false, false, 0, 0 };
            #endif
        }
        else {
            const auto is_dirty =
                le.state == state_type::invalid_dirty;
            
            // Change the state to read-only only when it was invalid.
            le.state = is_dirty ? state_type::readonly_dirty : state_type::readonly_clean;
            
            const auto cur_rd_ts = ge.rd_ts;
            
            #ifndef MEDSM2_FORCE_LATEST_READ
            if (acq_sig.is_valid_rd_ts(cur_rd_ts)) {
                // The home process written in this process is still valid.
                // (= before self-invalidation.)
                #ifdef MEDSM2_RELEASE_LATEST_READ
                return { true, false, is_dirty, le.home_proc, cur_rd_ts };
                #else
                return { true, is_dirty, le.home_proc, cur_rd_ts };
                #endif
            }
            else {
            #endif
                #ifdef MEDSM2_RELEASE_LATEST_READ
                return { true, true, is_dirty, le.home_proc, cur_rd_ts };
                #else
                const auto glk_ret =
                    this->lock_global(com, blk_pos, lk);
                
                // Create a new self-invalidation timestamp.
                // TODO: Provide a good prediction for each block.
                const auto new_rd_ts =
                    acq_sig.make_new_rd_ts(glk_ret.rd_ts);
                
                this->unlock_global_read(com, blk_pos, lk, glk_ret, new_rd_ts);
                
                ge.rd_ts = new_rd_ts;
                
                return { true, is_dirty, glk_ret.owner, new_rd_ts };
                #endif
            #ifndef MEDSM2_FORCE_LATEST_READ
            }
            #endif
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
    start_write_result start_write(const blk_pos_type blk_pos, const unique_lock_type& lk) {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        if (le.state == state_type::readonly_clean || le.state == state_type::readonly_dirty) {
            // Twinning is only required when only the private data is valid.
            const auto needs_twin =
                le.state == state_type::readonly_clean;
            
            // Change the state to writable only when it was read-only.
            le.state = state_type::writable;
            
            return { true, needs_twin, true };
        }
        else if (le.state == state_type::writable || le.state == state_type::released
            || le.state == state_type::pinned)
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
    
    MEFDN_NODISCARD
    bool try_set_released(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        // Upgrading is necessary before store-releasing.
        MEFDN_ASSERT(le.state != state_type::invalid);
        MEFDN_ASSERT(le.state != state_type::readonly);
        
        if (le.state == state_type::released) {
            // This block was store-released
            // and the releaser thread has not processed it.
            return false;
        }
        
        // Set the state to released in order to delay merging this block.
        le.state = state_type::released;
        
        return true;
    }
    
    void set_pinned(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        this->check_locked(blk_pos, lk);
        
        auto& le = this->les_[blk_pos];
        // Upgrading is necessary before store-releasing.
        MEFDN_ASSERT(le.state == state_type::writable);
        // TODO: Allow "released" blocks to be pinned.
        
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
        // This block must be write-protected (mprotect(PROT_READ))
        // before modifying the private data on this process.
        bool needs_protect_before;
        // If the block is invalid or read-only,
        // it should be removed from the write set.
        bool is_clean;
    };
    
    check_release_result check_release(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const bool              ordered
    ) {
        this->check_locked(blk_pos, lk);
        
        const auto& le = this->les_[blk_pos];
        const auto state = le.state;
        
        if (state == state_type::writable ||
            // Under the relaxed semantics, check if the block is marked as "released".
            (ordered && (state == state_type::released)))
        {
            // This block should be released now.
            return { true , true , false };
        }
        else if ((state == state_type::pinned) || (state == state_type::released)) {
            // Pinned/released blocks must not be released.
            throw std::logic_error("Releasing pinned/released is unsupported yet!");
            return { false, false, false };
        }
        else if (state == state_type::invalid_dirty || state == state_type::readonly_dirty) {
            // Although this block is invalid or readonly,
            // it's still dirty and must be released.
            return { true , false, true  };
        }
        else {
            // This block is already downgraded (invalid_clean or readonly_clean).
            return { false, false, true  };
        }
    }
    
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
        
        // Check whether the timestamp is newer or not.
        // FIXME: Consider timestamp overflow.
        if (ge.wr_ts < new_wr_ts) {
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
            else if (le.state == state_type::writable || le.state == state_type::released) {
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
            // Update the self-invalidation timestamp.
            // FIXME: Consider overwriting this member
            // when this block is writable on this process.
            ge.rd_ts = new_rd_ts;
            // Update the writer timestamp.
            ge.wr_ts = new_wr_ts;
        }
        
        return ret;
    }
    
    struct self_invalidate_result {
        bool        is_ignored;
        bool        needs_protect;
        bool        needs_merge;
        rd_ts_type  rd_ts;
    };
    
    self_invalidate_result self_invalidate(
        const acq_sig_type&     acq_sig
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        const auto ret = this->invalidate(blk_pos, lk, acq_sig.get_min_wr_ts());
        
        auto& ge = * this->ges_.local(blk_pos);
        const auto rd_ts = ge.rd_ts;
        
        return { ret.is_ignored, ret.needs_protect, ret.needs_merge, rd_ts };
    }
    
public:
    struct lock_global_result {
        proc_id_type    owner;
        rd_ts_type      rd_ts;
        wr_ts_type      wr_ts;
    };
    
    lock_global_result lock_global(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        
        // Load the probable owner from the "latest write notice".
        // Although it's possible to load the probable owner of the global entry,
        // the process which provided the write notice for the latest read
        // is more likely to be the current latest writer.
        proc_id_type prob_proc = this->les_[blk_pos].home_proc;
        
        while (true)
        {
            const auto ge_rptr = this->ges_.remote(prob_proc, blk_pos);
            
            const auto expected = this->make_owned_lock_val(prob_proc);
            const auto desired  = this->make_locked_lock_val(prob_proc);
            
            MEFDN_LOG_VERBOSE(
                "msg:Try to lock dir lock.\t"
                "blk_pos:{}\t"
                "prob_proc:{}"
            ,   blk_pos
            ,   prob_proc
            );
            
            // Try to lock the probable owner.
            const auto cas_result =
                rma.compare_and_swap(
                    prob_proc       // target_proc
                ,   &ge_rptr->lock  // target_rptr
                ,   expected        // expected
                ,   desired         // desired
                );
            
            if (cas_result == expected) {
                // The probable owner was exactly the latest owner.
                
                #ifdef MEDSM2_CHECK_VALID_LOCKED_LINKS
                // Validate "locked_val" of all of the processes.
                this->check_valid_locked_links(com, blk_pos, prob_proc);
                #endif
                
                // Load the latest timestamps.
                global_entry ge{};
                rma.read(prob_proc, ge_rptr, &ge, 1);
                // TODO: It's better if this read is overlapped with other communications.
                
                return { prob_proc, ge.rd_ts, ge.wr_ts };
            }
            else {
                // CAS failed because the probable owner was not the latest owner.
                // This thread starts to follow the distributed links only with read operations.
                while (true) {
                    const auto ge_rptr_2 = this->ges_.remote(prob_proc, blk_pos);
                    
                    atomic_int_type lock_val = 0;
                    rma.read(prob_proc, &ge_rptr_2->lock, &lock_val, 1);
                    
                    const auto expected_2 = this->make_owned_lock_val(prob_proc);
                    if (lock_val == expected_2) {
                        // The latest owner seems to allow the lock acquisition.
                        break;
                    }
                    
                    const auto locked = this->make_locked_lock_val(prob_proc);
                    if (lock_val == locked) {
                        // The latest owner is being locked now.
                        // This thread is busy-waiting for this lock
                        // and needs to forward the MPI progress engine.
                        rma.progress();
                    }
                    
                    // Follow the link written in the lock variable.
                    prob_proc = this->get_probable_owner_from_lock_val(prob_proc, lock_val);
                }
            }
            
            // Follow the link written in the lock variable.
            prob_proc = this->get_probable_owner_from_lock_val(prob_proc, cas_result);
        }
    }
    
    struct unlock_global_result {
        wr_ts_type     new_wr_ts;
    };
    
    template <typename MergeResult>
    unlock_global_result unlock_global(
        com_itf_type&               com
    ,   const blk_pos_type          blk_pos
    ,   const unique_lock_type&     lk
    ,   const lock_global_result&   glk_ret
    ,   const MergeResult&          mg_ret
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        
        const auto cur_proc = com.this_proc_id();
        const auto old_owner = glk_ret.owner;
        
        // If the data is written, the timestamp is updated.
        // If not, because the data is read from the old owner
        // (that will be the owner again),
        // the timestamp becomes equal to that of the owner.
        const auto new_wr_ts =
            mg_ret.is_written ? (glk_ret.wr_ts + 1) : glk_ret.wr_ts;
        
        const auto new_owner = mg_ret.is_migrated ? cur_proc : old_owner;
        
        auto& le = this->les_[blk_pos];
        auto& ge = * this->ges_.local(blk_pos);
        
        // Update the home process.
        // The home process is only updated locally.
        le.home_proc = new_owner;
        
        // Update the write timestamp.
        // Although this value may be read by another writer,
        // this process still has the lock for it.
        // Also, the write timestamp only increases monotonically.
        ge.wr_ts = new_wr_ts;
        
        // There are 3 conditions:
        // (1) This process was the (old) owner and is still the (new) owner.
        //      is_migrated == false, old_owner == new_owner == cur_proc
        // (2) The remote process was the (old) owner and is still the (new) owner.
        //      is_migrated == false, old_owner == new_owner != cur_proc
        // (3) The remote process was the (old) owner, but this process became a new owner.
        //      is_migrated == true, old_owner != cur_proc, new_owner == cur_proc
        //
        // It is unnecessary to consider (4) where
        //  "this process was the (old) owner, but the remote process became a new owner"
        // because this process is currently releasing this block.
        
        if (old_owner != cur_proc) {
            // (2) (3) The remote process was the (old) owner.
            
            // (2) linked_proc == cur_proc != old_owner == new_owner
            // (3) linked_proc == old_owner != cur_proc == new_owner
            const auto linked_proc =
                mg_ret.is_migrated ? /*(3)*/ old_owner : /*(2)*/ cur_proc;
            
            const auto ge_rptr = this->ges_.remote(linked_proc, blk_pos);
            const auto new_lock_val =
                this->make_linked_lock_val(new_owner /* == (2) old_owner, (3) cur_proc */);
            
            // Assign a link to the non-owner process.
            rma.write(linked_proc, &ge_rptr->lock, &new_lock_val, 1);
            
            // In (2), this is simply replacing the existing link to a newer one.
            // In (3), this will create a circular dependency temporarily.
        }
        
        {
            const auto ge_rptr = this->ges_.remote(new_owner, blk_pos);
            const auto new_lock_val = this->make_owned_lock_val(new_owner);
            
            // Set the new owner as unlocked.
            rma.write(new_owner, &ge_rptr->lock, &new_lock_val, 1);
        }
        
        const auto old_state = le.state;
        auto new_state = old_state;
        
        // TODO: Invalid blocks are locked when the read timestamp is too old.
        //MEFDN_ASSERT(old_state != state_type::invalid_clean);
        //MEFDN_ASSERT(old_state != state_type::readonly_clean);
        MEFDN_ASSERT(old_state != state_type::released);
        MEFDN_ASSERT(old_state != state_type::pinned);
        
        if (old_state == state_type::invalid_dirty) {
            new_state = state_type::invalid_clean;
        }
        else if (old_state == state_type::readonly_dirty) {
            new_state = state_type::readonly_clean;
        }
        else if (old_state == state_type::writable) {
            // Check that cur_proc != old_owner.
            if (mg_ret.becomes_clean) {
                // This block was downgraded to read-only
                // because the old owner wrote on this block.
                // The state is updated because of this downgrading.
                new_state = state_type::readonly_clean;
            }
            else {
                // This block was not write-protected in this release
                // and still can be written in this process.
                new_state = state_type::writable;
            }
        }
        
        le.state = new_state;
        
        return { new_wr_ts };
    }
    
    #ifndef MEDSM2_RELEASE_LATEST_READ
private:
    void unlock_global_read(
        com_itf_type&               com
    ,   const blk_pos_type          blk_pos
    ,   const unique_lock_type&     lk
    ,   const lock_global_result&   glk_ret
    ,   const rd_ts_type            new_rd_ts
    ) {
        this->check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        
        const auto owner = glk_ret.owner;
        
        {
            const auto ge_rptr = this->ges_.remote(owner, blk_pos);
            
            // Write a new self-invalidation timestamp.
            rma.write(owner, &ge_rptr->rd_ts, &new_rd_ts, 1);
        }
        
        {
            const auto ge_rptr = this->ges_.remote(owner, blk_pos);
            const auto new_lock_val = this->make_owned_lock_val(owner);
            
            // Set the owner as unlocked.
            rma.write(owner, &ge_rptr->lock, &new_lock_val, 1);
        }
    }
    #endif
    
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
    static atomic_int_type make_owned_lock_val(const proc_id_type /*owner*/) {
        // 1 means "owned".
        return 1;
    }
    static atomic_int_type make_locked_lock_val(const proc_id_type /*owner*/) {
        // 2 means "locked".
        return 2;
    }
    static atomic_int_type make_linked_lock_val(const proc_id_type owner) {
        // Other processes are expressed without 0, 1 & 2.
        return static_cast<atomic_int_type>(owner) + 3;
    }
    static bool is_valid_linked_lock_val(
        const proc_id_type      num_procs
    ,   const proc_id_type      lock_val_proc
    ,   const atomic_int_type   lock_val
    ) {
        return 3 <= lock_val && lock_val < (3+num_procs)
            // Loop should be expressed as "owned" or "locked" instead.
            && lock_val != (3+lock_val_proc);
    }
    static proc_id_type get_probable_owner_from_lock_val(
        const proc_id_type      lock_val_proc
    ,   const atomic_int_type   lock_val
    ) {
        MEFDN_ASSERT(lock_val != 0);
        
        return
            // Check whether the process of the lock value is the current owner.
            lock_val <= 2
            // If so, simply return the process ID.
        ?   lock_val_proc
            // Otherwise, do the inverse of make_linked_lock_val.
        :   static_cast<proc_id_type>(lock_val - 3);
    }
    
    void check_valid_locked_links(
        com_itf_type&       com
    ,   const blk_pos_type  blk_pos
    ,   const proc_id_type  locked_proc
    ) {
        // This function can only work when this process is holding the global lock.
        
        auto& rma = com.get_rma();
        //const auto this_proc = com.this_proc_id();
        const auto num_procs = com.get_num_procs();
        
        const auto links_buf =
            rma.template make_unique<atomic_int_type []>(num_procs);
        
        const auto links = links_buf.get();
        
        // Load the "lock" values on all of the processes.
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            const auto ge_rptr = this->ges_.remote(proc, blk_pos);
            rma.read(proc, &ge_rptr->lock, links + proc, 1);
        }
        
        auto&& produce_error = [&] (const char* const msg) {
            fmt::MemoryWriter w;
            w.write("msg:Error of probable owners ({}).\t", msg);
            w.write("blk_pos:{}\t", blk_pos);
            w.write("locked_proc:{}\t", locked_proc);
            w.write("locked_vals:");
            for (proc_id_type proc = 0; proc < num_procs; ++proc) {
                w.write("{},", links[proc]);
            }
            
            const auto s = w.str();
            MEFDN_LOG_FATAL("{}", s);
            throw std::logic_error(s);
        };
        
        // Validate that there is only one "locked" value.
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            const auto lock_val = links[proc];
            if (proc == locked_proc) {
                if (lock_val != make_locked_lock_val(proc)) {
                    produce_error("locked_proc doesn't have lock");
                }
            }
            else {
                if (!is_valid_linked_lock_val(num_procs, proc, lock_val)) {
                    produce_error("invalid link");
                }
            }
        }
        
        // Validate that there is no circular links.
        for (proc_id_type proc_start = 0; proc_start < num_procs; ++proc_start) {
            // Avoid using std::vector<bool> (the performance is not serious here, though).
            mefdn::vector<int> is_visited(num_procs, 0);
            
            proc_id_type proc = proc_start;
            while (true) {
                const auto lock_val = links[proc];
                if (lock_val == make_locked_lock_val(proc)) {
                    // OK, this process reached the locked val.
                    MEFDN_ASSERT(proc == locked_proc);
                    break;
                }
                
                is_visited[proc] = true;
                
                MEFDN_ASSERT(is_valid_linked_lock_val(num_procs, proc, lock_val));
                proc = get_probable_owner_from_lock_val(proc, lock_val);
                
                if (is_visited[proc]) {
                    produce_error("circular link detected");
                }
            }
        }
    }
    
private:
    struct global_entry {
        // A probable owner ID + a lock bit (LSB).
        atomic_int_type lock;
        // The timestamp when this block must be self-invalidated.
        // If this process is the owner of this block,
        // this member may be modified by other reader processes.
        rd_ts_type  rd_ts;
        // The timestamp that can be read by other writers.
        // Only modified by the local process.
        wr_ts_type  wr_ts;
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

