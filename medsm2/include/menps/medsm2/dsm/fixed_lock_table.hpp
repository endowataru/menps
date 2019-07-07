
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class fixed_lock_table
{
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using p2p_itf_type = typename com_itf_type::p2p_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using lock_pos_type = typename P::lock_pos_type;
    using p2p_tag_type = typename P::p2p_tag_type;
    using atomic_int_type = typename P::atomic_int_type;
    using size_type = typename P::size_type;
    
    using local_byte_ptr_type =
        typename rma_itf_type::template local_ptr<mefdn::byte>;
    using remote_byte_ptr_type =
        typename rma_itf_type::template remote_ptr<mefdn::byte>;
    using remote_aint_ptr_type =
        typename rma_itf_type::template remote_ptr<atomic_int_type>;
    
public:
    void coll_make(
        com_itf_type&   com
    ,   const size_type num_lks
    ,   const size_type lad_size
    ) {
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        
        const auto num_procs = coll.get_num_procs();
        
        this->num_lks_ = num_lks;
        this->num_lks_per_proc_ =
            mefdn::roundup_divide(num_lks, static_cast<size_type>(num_procs));
        this->lad_size_ = lad_size;
        
        this->glks_.coll_make(rma, coll, this->num_lks_per_proc_);
        if (lad_size != 0) {
            this->lads_.coll_make(rma, coll, this->num_lks_per_proc_ * lad_size);
        }
    }
    
    struct lock_global_result {
        // The latest owner's ID.
        proc_id_type    owner;
    };
    
    lock_global_result lock_global(
        com_itf_type&           com
    ,   p2p_itf_type&           p2p
    ,   const lock_pos_type     lk_pos
    ,   const p2p_tag_type      /*tag*/
    ,   local_byte_ptr_type     lad_buf
    ) {
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        const auto ll = this->get_lock_location(com, lk_pos);
        
        while (true)
        {
            const atomic_int_type expected = 0;
            const atomic_int_type desired  = 1;
            
            // Try to lock.
            const auto cas_result =
                rma.compare_and_swap(
                    ll.proc     // target_proc
                ,   ll.lk_rptr  // target_rptr
                ,   expected    // expected
                ,   desired     // desired
                );
            
            if (cas_result == expected) {
                // This thread successfully acquired the lock.
                break;
            }
            
            // This thread is busy-waiting for this lock
            // and needs to forward the MPI progress engine.
            rma.progress();
        }
        
        // Read the LAD.
        rma.read(
            ll.proc
        ,   ll.lad_rptr
        ,   lad_buf
        ,   ll.lad_size
        );
        
        // Acquired the lock.
        MEFDN_LOG_VERBOSE(
            "msg:Acquired lock.\t"
            "lk_pos:{}\t"
            "proc:{}"
        ,   lk_pos
        ,   ll.proc
        );
        
        return { ll.proc };
    }
    
    void unlock_global(
        com_itf_type&           com
    ,   p2p_itf_type&           p2p
    ,   const lock_pos_type     lk_pos
    ,   const p2p_tag_type      /*tag*/
    ,   const void* const       lad_ptr
    ) {
        auto& rma = com.get_rma();
        const auto this_proc = com.this_proc_id();
        
        const auto ll = this->get_lock_location(com, lk_pos);
        
        const auto lad_bptr = static_cast<const mefdn::byte*>(lad_ptr);
        
        // Read the LAD.
        // TODO: Remove buffering.
        rma.buf_write(
            ll.proc
        ,   ll.lad_rptr
        ,   lad_bptr
        ,   ll.lad_size
        );
        
        // Flush the writes before unlocking.
        rma.flush(ll.proc);
        
        const atomic_int_type zero = 0;
        
        // Unlock by writing zero to the lock variable.
        // TODO: Use atomic write (MPI_Accumulate(MPI_REPLACE)).
        rma.buf_write(ll.proc, ll.lk_rptr, &zero, 1);
    }
    
private:
    struct lock_location {
        proc_id_type            proc;
        remote_aint_ptr_type    lk_rptr;
        remote_byte_ptr_type    lad_rptr;
        size_type               lad_size;
    };
    
    lock_location get_lock_location(
        com_itf_type&       com
    ,   const lock_pos_type lk_pos
    ) {
        const auto num_procs = com.get_num_procs();
        const auto lk_idx  = lk_pos / num_procs;
        const auto proc_id = lk_pos % num_procs;
        const auto lad_size = this->lad_size_;
        
        return {
            proc_id
        ,   this->glks_.remote(proc_id, lk_idx)
        ,   this->lads_.remote(proc_id, lk_idx * lad_size)
        ,   lad_size
        };
    }
    
    typename P::template alltoall_buffer<atomic_int_type>   glks_;
    typename P::template alltoall_buffer<mefdn::byte>       lads_;
    size_type num_lks_ = 0;
    size_type num_lks_per_proc_ = 0;
    size_type lad_size_ = 0;
};

} // namespace medsm2
} // namespace menps


