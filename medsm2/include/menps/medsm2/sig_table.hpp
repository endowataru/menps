
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class sig_table
{
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using atomic_int_type = typename P::atomic_int_type;
    using size_type = typename P::size_type;
    
    using aint_rptr_type =
        typename rma_itf_type::template remote_ptr<atomic_int_type>;
    
    using byte_rptr_type =
        typename rma_itf_type::template remote_ptr<mefdn::byte>;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    using sig_id_type = typename P::sig_id_type;
    
public:
    template <typename Conf>
    void coll_init(const Conf& conf)
    {
        auto& com = conf.com;
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        
        this->num_sigs_ = conf.max_num_sigs;
        this->sig_bytes_ = conf.sig_size_in_bytes;
        
        this->lk_buf_.coll_make(rma, coll, this->num_sigs_);
        this->sig_buf_.coll_make(rma, coll, this->num_sigs_ * this->sig_bytes_);
    }
    
    sig_buffer_type get_sig(
        com_itf_type&       com
    ,   const sig_id_type   sig_id
    ) {
        const auto sl = this->get_sig_location(com, sig_id);
        
        // Lock the signature.
        this->lock(com, sl);
        
        auto& rma = com.get_rma();
        
        // Allocate a temporary buffer for reading the signature.
        // TODO: Reuse this buffer.
        const auto buf =
            rma.template make_unique_uninitialized<mefdn::byte []>(sl.sig_bytes);
        
        const auto buf_ptr = buf.get();
        
        // Read the signature data.
        rma.read(
            sl.proc
        ,   sl.sig_rptr
        ,   buf_ptr
        ,   sl.sig_bytes
        );
        
        // Unlock the signature.
        this->unlock(com, sl);
        
        // Deserialize the buffer and return it as a signature.
        return sig_buffer_type::deserialize_from(buf_ptr, sl.sig_bytes);
        
        // The temporary buffer is automatically deleted here.
    }
    
    void merge_sig_to(
        com_itf_type&           com
    ,   const sig_id_type       sig_id
    ,   const sig_buffer_type&  sig_buf
    ) {
        const auto sl = this->get_sig_location(com, sig_id);
        
        // Lock the signature.
        this->lock(com, sl);
        
        auto& rma = com.get_rma();
        
        // Allocate a temporary buffer for reading the signature.
        // TODO: Reuse this buffer.
        const auto buf =
            rma.template make_unique_uninitialized<mefdn::byte []>(sl.sig_bytes);
        
        const auto buf_ptr = buf.get();
        
        // Read the signature data.
        rma.read(
            sl.proc
        ,   sl.sig_rptr
        ,   buf_ptr
        ,   sl.sig_bytes
        );
        
        // Deserialize the buffer.
        const auto tbl_sig = sig_buffer_type::deserialize_from(buf_ptr, sl.sig_bytes);
        
        // Merge two buffers.
        const auto merged_sig =
            sig_buffer_type::merge(tbl_sig, sig_buf);
        
        // Serialize the buffer again.
        const auto ser_buf = merged_sig.serialize(sl.sig_bytes);
        
        // Write the signature data.
        rma.write(
            sl.proc
        ,   sl.sig_rptr
        ,   ser_buf.get()
        ,   sl.sig_bytes
        );
        
        // Unlock the signature.
        this->unlock(com, sl);
        
        // The temporary buffer is automatically deleted here.
    }
    
private:
    struct sig_location {
        proc_id_type    proc;
        aint_rptr_type  lk_rptr;
        byte_rptr_type  sig_rptr;
        size_type       sig_bytes;
    };
    
    sig_location get_sig_location(
        com_itf_type&       com
    ,   const sig_id_type   sig_id
    ) {
        const auto num_procs = com.get_num_procs();
        const auto proc_id = sig_id % num_procs;
        // sig_pos must be smaller than this->num_sigs_.
        const auto sig_pos = sig_id / num_procs % this->num_sigs_;
        
        return {
            proc_id
        ,   lk_buf_.remote(proc_id, sig_pos)
        ,   sig_buf_.remote(proc_id, sig_pos * this->sig_bytes_)
        ,   this->sig_bytes_
        };
    }
    
    void lock(
        com_itf_type&       com
    ,   const sig_location& sl
    ) {
        auto& rma = com.get_rma();
        
        while (true)
        {
            const atomic_int_type expected = 0;
            const atomic_int_type desired  = 1;
            
            // Try to lock.
            const auto cas_result =
                rma.compare_and_swap(
                    sl.proc     // target_proc
                ,   sl.lk_rptr  // target_rptr
                ,   expected    // expected
                ,   desired     // desired
                );
            
            if (cas_result == desired) {
                // This thread successfully acquired the lock.
                break;
            }
            
            // This thread is busy-waiting for this lock
            // and needs to forward the MPI progress engine.
            rma.progress();
        }
    }
    
    void unlock(
        com_itf_type&       com
    ,   const sig_location& sl
    ) {
        auto& rma = com.get_rma();
        
        const atomic_int_type zero = 0;
        
        // Unlock by writing zero to the lock variable.
        // TODO: Use atomic write (MPI_Accumulate(MPI_REPLACE)).
        rma.write(sl.proc, sl.lk_rptr, &zero, 1);
    }
    
    size_type num_sigs_ = 0;
    size_type sig_bytes_ = 0;
    
    typename P::template alltoall_buffer<atomic_int_type>
        lk_buf_;
    
    typename P::template alltoall_buffer<mefdn::byte>
        sig_buf_;
};

} // namespace medsm2
} // namespace menps

