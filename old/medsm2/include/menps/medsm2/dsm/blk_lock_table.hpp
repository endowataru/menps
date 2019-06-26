
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/medsm2/prof.hpp>

//#define MEDSM2_CHECK_VALID_LOCKED_LINKS

namespace menps {
namespace medsm2 {

template <typename P>
class blk_lock_table
{
    MEFDN_DEFINE_DERIVED(P)
    
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    
    using unique_lock_type = typename P::unique_lock_type;
    
public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        auto& coll = conf.com.get_coll();
        const auto num_blks = conf.num_blks;
        
        this->glks_.coll_make(conf.com.get_rma(), coll, num_blks);
        
        const auto this_proc = coll.this_proc_id();
        const auto num_procs = coll.get_num_procs();
        
        for (blk_pos_type blk_pos = 0; blk_pos < num_blks; ++blk_pos) {
            const auto local_glk = this->glks_.local(blk_pos);
            
            // The owner of blk_pos is (blk_pos % num_procs).
            const auto owner =
                static_cast<proc_id_type>(
                    blk_pos % static_cast<blk_pos_type>(num_procs)
                );
            
            const auto lock_val =
                    owner == this_proc
                ?   this->make_owned_lock_val(owner)
                :   this->make_linked_lock_val(owner);
            
            *local_glk = lock_val;
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
    
    struct lock_global_result {
        // The latest owner's ID.
        proc_id_type    owner;
    };
    
    lock_global_result lock_global(
        com_itf_type&           com
    ,   const blk_id_type       blk_id
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(blk_pos);
        
        #ifdef MEDSM2_ENABLE_P2P_LOCK
        // Read the local lock value first.
        // TODO: This load must be an "atomic load" in MPI RMA.
        //       Replace with MPI_Fetch_and_op(MPI_NO_OP).
        auto link_val = *local_glk;
        
        // It is a bug if this process tries to lock again.
        MEFDN_ASSERT(link_val != this->make_locked_lock_val(this_proc));
        
        // If this process seems to be the last releaser,
        // the lock value must be atomically exchanged to "locked"
        // because other processes may also replace it concurrently.
        if (link_val == this->make_owned_lock_val(this_proc))
        {
            const auto local_glk_rptr =
                this->glks_.remote(this_proc, blk_pos);
            
            const auto desired =
                this->make_locked_lock_val(this_proc);
            
            const auto cas_result =
                rma.compare_and_swap(
                    this_proc       // target_proc
                ,   local_glk_rptr  // target_rptr
                ,   link_val        // expected
                ,   desired         // desired
                );
            
            if (cas_result == link_val) {
                // Acquired the lock.
                // This process is both the last releaser and the owner.
                return { this_proc };
            }
            
            link_val = cas_result;
        }
        
        // Set the local lock value to "locked".
        // TODO: Replace with MPI_Fetch_and_op(MPI_REPLACE).
        *local_glk = this->make_locked_lock_val(this_proc);
        
        auto proc = this_proc;
        
        while (true)
        {
            const auto next_proc = this->get_probable_owner_from_lock_val(proc, link_val);
            
            MEFDN_LOG_VERBOSE(
                "msg:Follow probable owners.\t"
                "blk_pos:{}\t"
                "prev_proc:{}\t"
                "next_proc:{}\t"
                "link_val:{}"
            ,   blk_pos
            ,   proc
            ,   next_proc
            ,   link_val
            );
            
            //MEFDN_ASSERT(next_proc != proc);
            
            proc = next_proc;
            
            const auto glk_rptr = this->glks_.remote(proc, blk_pos);
            
            auto expected = this->make_owned_lock_val(proc);
            const auto desired = this->make_linked_lock_val(this_proc);
            
            link_val = rma.compare_and_swap(proc, glk_rptr, expected, desired);
            
            if (link_val == expected) {
                // Acquired the lock.
                MEFDN_LOG_VERBOSE(
                    "msg:Acquired lock.\t"
                    "blk_pos:{}\t"
                    "proc:{}"
                ,   blk_pos
                ,   proc
                );
                return { proc };
            }
            
            expected = this->make_locked_lock_val(proc);
            
            if (link_val == expected) {
                // Try to follow the last releaser and become the next.
                link_val = rma.compare_and_swap(proc, glk_rptr, expected, desired);
                
                if (link_val == expected) {
                    auto& p2p = com.get_p2p();
                    const auto tag = P::get_tag_from_blk_id(blk_id);
                    
                    MEFDN_LOG_VERBOSE(
                        "msg:Wait for the previous releaser.\t"
                        "blk_pos:{}\t"
                        "proc:{}"
                    ,   blk_pos
                    ,   proc
                    );
                    
                    // Waiting from the same process is a bug.
                    MEFDN_ASSERT(proc != this_proc);
                    
                    // Wait for the previous releaser.
                    p2p.untyped_recv(proc, tag, nullptr, 0);
                    
                    MEFDN_LOG_VERBOSE(
                        "msg:Acquired transferred lock.\t"
                        "blk_pos:{}\t"
                        "proc:{}"
                    ,   blk_pos
                    ,   proc
                    );
                    
                    // FIXME: This ID is not returned by "end_transaction" of the last releaser.
                    return { proc };
                }
            }
        }
        
        #else
        // Start from the probable owner.
        // TODO: The process which provided the write notice for the latest read
        //       may be a more recent writer.
        //       The previous implementation used it.
        auto prob_proc =
            this->get_probable_owner_from_lock_val(this_proc, *local_glk);
        
        while (true)
        {
            const auto glk_rptr = this->glks_.remote(prob_proc, blk_pos);
            
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
                    prob_proc   // target_proc
                ,   glk_rptr    // target_rptr
                ,   expected    // expected
                ,   desired     // desired
                );
            
            if (cas_result == expected) {
                // The probable owner was exactly the latest owner.
                break;
            }
            else {
                // CAS failed because the probable owner was not the latest owner.
                // This thread starts to follow the distributed links only with read operations.
                while (true) {
                    const auto glk_rptr_2 = this->glks_.remote(prob_proc, blk_pos);
                    
                    // Read the link.
                    // TODO: Reduce this allocation.
                    const auto lock_buf =
                        rma.buf_read(prob_proc, glk_rptr_2, 1);
                    
                    const auto lock_val = *lock_buf;
                    
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
        
        #ifdef MEDSM2_CHECK_VALID_LOCKED_LINKS
        // Validate "locked_val" of all of the processes.
        this->check_valid_locked_links(com, blk_pos, prob_proc);
        #endif
        
        return { prob_proc };
        
        #endif
    }
    
    template <typename EndTransactionResult>
    void unlock_global(
        com_itf_type&               com
    ,   const blk_id_type           blk_id
    ,   const blk_pos_type          blk_pos
    ,   const unique_lock_type&     lk
    ,   const lock_global_result&   glk_ret MEFDN_MAYBE_UNUSED
    ,   const EndTransactionResult& et_ret MEFDN_MAYBE_UNUSED
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        auto& rma = com.get_rma();
        
        #ifdef MEDSM2_ENABLE_LAZY_MERGE
        // Complete writing on the previous owner.
        rma.flush(glk_ret.owner);
        #endif
        
        #ifdef MEDSM2_ENABLE_P2P_LOCK
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(blk_pos);
        
        auto link_val = *local_glk;
        
        // It is a bug if this process has already unlocked.
        MEFDN_ASSERT(link_val != this->make_owned_lock_val(this_proc));
        
        if (link_val == this->make_locked_lock_val(this_proc))
        {
            const auto local_glk_rptr =
                this->glks_.remote(this_proc, blk_pos);
            
            const auto desired =
                this->make_owned_lock_val(this_proc);
            
            const auto cas_result =
                rma.compare_and_swap(
                    this_proc       // target_proc
                ,   local_glk_rptr  // target_rptr
                ,   link_val        // expected
                ,   desired         // desired
                );
            
            if (cas_result == link_val) {
                // Released the lock.
                return;
            }
            
            link_val = cas_result;
        }
        
        auto& p2p = com.get_p2p();
        const auto tag = P::get_tag_from_blk_id(blk_id);
        
        const auto next_proc =
            this->get_probable_owner_from_lock_val(this_proc, link_val);
        
        // Transfer the lock to the next acquirer.
        p2p.untyped_send(next_proc, tag, nullptr, 0);
        
        #else
        
        const auto cur_proc = com.this_proc_id();
        const auto old_owner = glk_ret.owner;
        
        const auto new_owner = et_ret.new_owner;
        
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
        
        if (new_owner == cur_proc) {
            // This process is the new owner.
            // (1) old_owner == new_owner == cur_proc
            // (3) old_owner != cur_proc == new_owner
            
            // TODO: Call MPI_Win_sync() ?
            
            if (old_owner != cur_proc) {
                // (3) old_owner != cur_proc == new_owner
                
                const auto glk_rptr =
                    this->glks_.remote(old_owner, blk_pos);
                
                const auto new_lock_val =
                    this->make_linked_lock_val(new_owner /* == cur_proc */);
                
                // Assign a link to the old owner process.
                // TODO: Reduce this allocation.
                rma.buf_write(old_owner, glk_rptr, &new_lock_val, 1);
                
                // TODO: This will create a circular dependency temporarily.
            }
            
            {
                const auto local_glk = this->glks_.local(blk_pos);
                
                const auto new_lock_val =
                    this->make_owned_lock_val(new_owner /* == cur_proc */);
                
                // Set the new owner (= this process) as unlocked.
                *local_glk = new_lock_val;
            }
        }
        else {
            // This process is not the new owner.
            // (2) old_owner == new_owner != cur_proc
            
            MEFDN_ASSERT(new_owner == old_owner);
            
            const auto glk_rptr =
                this->glks_.remote(new_owner /* == old_owner */, blk_pos);
            
            const auto new_lock_val =
                this->make_owned_lock_val(new_owner /* == old_owner */);
            
            // Set the owner as unlocked.
            rma.buf_write(new_owner /* == old_owner */, glk_rptr, &new_lock_val, 1);
        }
        #endif
    }
    
    bool check_owned(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(blk_pos);
        
        // Load the lock value of this process.
        const auto lock_val = *local_glk;
        
        // This process isn't locking this block.
        MEFDN_ASSERT(lock_val != this->make_locked_lock_val(this_proc));
        
        const auto ret = lock_val == this->make_owned_lock_val(this_proc);
        if (!ret) {
            prof::add(prof_kind::tx_migrate, 1);
        }
        return ret;
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
        const auto num_procs = com.get_num_procs();
        
        const auto links_buf =
            rma.template make_unique<atomic_int_type []>(num_procs);
        
        const auto links = links_buf.get();
        
        // Load the "lock" values on all of the processes.
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            const auto glk_rptr = this->glks_.remote(proc, blk_pos);
            rma.read(proc, glk_rptr, links + proc, 1);
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
    typename P::template
        alltoall_buffer<atomic_int_type> glks_;
};

} // namespace medsm2
} // namespace menps

