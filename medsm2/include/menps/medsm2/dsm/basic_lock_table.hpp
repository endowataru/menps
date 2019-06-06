
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class basic_lock_table
{
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    using size_type = typename P::size_type;
    
    using lock_pos_type = typename P::lock_pos_type;
    
    using p2p_tag_type = typename P::p2p_tag_type;
    
public:
    void coll_make(
        com_itf_type&   com
    ,   const size_type num_lks
    ) {
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        
        this->glks_.coll_make(rma, coll, num_lks);
        
        const auto this_proc = coll.this_proc_id();
        const auto num_procs = coll.get_num_procs();
        
        for (lock_pos_type lk_pos = 0; lk_pos < num_lks; ++lk_pos) {
            const auto local_glk = this->glks_.local(lk_pos);
            
            // The owner of lk_pos is (lk_pos % num_procs).
            const auto owner =
                static_cast<proc_id_type>(
                    lk_pos % static_cast<lock_pos_type>(num_procs)
                );
            
            const auto lock_val =
                    owner == this_proc
                ?   this->make_owned_lock_val(owner)
                :   this->make_linked_lock_val(owner);
            
            *local_glk = lock_val;
        }
    }
    
    struct lock_global_result {
        // The latest owner's ID.
        proc_id_type    owner;
    };
    
    lock_global_result lock_global(
        com_itf_type&           com
    ,   const lock_pos_type     lk_pos
    ,   p2p_tag_type            tag
    ) {
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(lk_pos);
        
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
                this->glks_.remote(this_proc, lk_pos);
            
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
                "lk_pos:{}\t"
                "prev_proc:{}\t"
                "next_proc:{}\t"
                "link_val:{}"
            ,   lk_pos
            ,   proc
            ,   next_proc
            ,   link_val
            );
            
            proc = next_proc;
            
            const auto glk_rptr = this->glks_.remote(proc, lk_pos);
            
            auto expected = this->make_owned_lock_val(proc);
            const auto desired = this->make_linked_lock_val(this_proc);
            
            link_val = rma.compare_and_swap(proc, glk_rptr, expected, desired);
            
            if (link_val == expected) {
                // Acquired the lock.
                MEFDN_LOG_VERBOSE(
                    "msg:Acquired lock.\t"
                    "lk_pos:{}\t"
                    "proc:{}"
                ,   lk_pos
                ,   proc
                );
                return { proc };
            }
            
            expected = this->make_locked_lock_val(proc);
            
            if (link_val == expected) {
                // Try to follow the last releaser and become the next.
                link_val = rma.compare_and_swap(proc, glk_rptr, expected, desired);
                
                if (link_val == expected) {
                    auto& p2p = com.get_p2p_lock();
                    
                    MEFDN_LOG_VERBOSE(
                        "msg:Wait for the previous releaser.\t"
                        "lk_pos:{}\t"
                        "proc:{}"
                    ,   lk_pos
                    ,   proc
                    );
                    
                    // Waiting from the same process is a bug.
                    MEFDN_ASSERT(proc != this_proc);
                    
                    // Wait for the previous releaser.
                    p2p.untyped_recv(proc, tag, nullptr, 0);
                    
                    MEFDN_LOG_VERBOSE(
                        "msg:Acquired transferred lock.\t"
                        "lk_pos:{}\t"
                        "proc:{}"
                    ,   lk_pos
                    ,   proc
                    );
                    
                    // FIXME: This ID is not returned by "end_transaction" of the last releaser.
                    return { proc };
                }
            }
        }
    }
    
    void unlock_global(
        com_itf_type&           com
    ,   const lock_pos_type     lk_pos
    ,   p2p_tag_type            tag
    ) {
        auto& rma = com.get_rma();
        
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(lk_pos);
        
        auto link_val = *local_glk;
        
        // It is a bug if this process has already unlocked.
        MEFDN_ASSERT(link_val != this->make_owned_lock_val(this_proc));
        
        if (link_val == this->make_locked_lock_val(this_proc))
        {
            const auto local_glk_rptr =
                this->glks_.remote(this_proc, lk_pos);
            
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
        
        auto& p2p = com.get_p2p_lock();
        
        const auto next_proc =
            this->get_probable_owner_from_lock_val(this_proc, link_val);
        
        // Transfer the lock to the next acquirer.
        p2p.untyped_send(next_proc, tag, nullptr, 0);
    }
    
    bool check_owned(
        com_itf_type&           com
    ,   const lock_pos_type     lk_pos
    ) {
        const auto this_proc = com.this_proc_id();
        
        const auto local_glk = this->glks_.local(lk_pos);
        
        // Load the lock value of this process.
        const auto lock_val = *local_glk;
        
        // This process isn't locking this block.
        MEFDN_ASSERT(lock_val != this->make_locked_lock_val(this_proc));
        
        return lock_val == this->make_owned_lock_val(this_proc);
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
    
private:
    typename P::template
        alltoall_buffer<atomic_int_type> glks_;
};

} // namespace medsm2
} // namespace menps

