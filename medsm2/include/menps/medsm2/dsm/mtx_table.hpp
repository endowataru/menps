
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class mtx_table
{
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using size_type = typename P::size_type;
    
    using mtx_id_type = typename P::mtx_id_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    
    using lock_table_type = typename P::lock_table_type;
    
    using byte_rptr_type =
        typename rma_itf_type::template remote_ptr<mefdn::byte>;
    
    using mutex_type = typename P::mutex_type;
    
public:
    template <typename Conf>
    void coll_init(const Conf& conf)
    {
        auto& com = conf.com;
        #ifndef MEDSM2_USE_LAD
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        #endif
        
        this->num_locks_ = conf.max_num_locks;
        this->sig_bytes_ = conf.sig_size_in_bytes;
        
        this->les_ = mefdn::make_unique<local_entry []>(this->num_locks_);
        
        #ifdef MEDSM2_USE_LAD
        this->lk_tbl_.coll_make(com, this->num_locks_, this->sig_bytes_);
        
        #else
        this->lk_tbl_.coll_make(com, this->num_locks_);
        
        this->sig_buf_.coll_make(rma, coll, this->num_locks_ * this->sig_bytes_);
        #endif
    }
    
    struct lock_result
    {
        sig_buffer_type sig_buf;
    };
    
    lock_result lock(
        com_itf_type&       com
    ,   const mtx_id_type   mtx_id
    ) {
        auto& rma = com.get_rma();
        
        auto& le = this->les_[mtx_id];
        le.mtx.lock();
        
        auto& p2p = com.get_p2p_lock();
        const auto tag = P::get_tag_from_lock_id(mtx_id);
        
        #ifdef MEDSM2_USE_LAD
        const auto sig_buf_lptr =
            rma.template make_unique_uninitialized<mefdn::byte []>(this->sig_bytes_);
        
        const auto buf_ptr = sig_buf_lptr.get();
        
        this->lk_tbl_.lock_global(com, p2p, mtx_id, tag, buf_ptr);
        
        #else
        this->lk_tbl_.lock_global(com, p2p, mtx_id, tag);
        
        const auto sl = this->get_sig_location(com, mtx_id);
        
        // Read the signature data.
        // TODO: Reduce this allocation.
        const auto buf =
            rma.buf_read(
                sl.proc
            ,   sl.sig_rptr
            ,   sl.sig_bytes
            );
        
        const auto buf_ptr = buf.get();
        #endif
        
        // Deserialize the buffer and return it as a signature.
        auto sig_buf = sig_buffer_type::deserialize_from(buf_ptr, this->sig_bytes_);
        
        return { mefdn::move(sig_buf) };
    }
    
    void unlock(
        com_itf_type&           com
    ,   const mtx_id_type       mtx_id
    ,   const sig_buffer_type&  sig_buf
    ) {
        #ifndef MEDSM2_USE_LAD
        auto& rma = com.get_rma();
        
        const auto sl = this->get_sig_location(com, mtx_id);
        #endif
        
        const auto ser_buf = sig_buf.serialize(this->sig_bytes_);
        
        #ifndef MEDSM2_USE_LAD
        // Write the signature data.
        rma.buf_write(
            sl.proc
        ,   sl.sig_rptr
        ,   ser_buf.get()
        ,   sl.sig_bytes
        );
        
        // Complete writing on the previous owner.
        rma.flush(sl.proc);
        #endif
        
        auto& p2p = com.get_p2p_lock();
        const auto tag = P::get_tag_from_lock_id(mtx_id);
        
        #ifdef MEDSM2_USE_LAD
        this->lk_tbl_.unlock_global(com, p2p, mtx_id, tag, ser_buf.get());
        #else
        this->lk_tbl_.unlock_global(com, p2p, mtx_id, tag);
        #endif
        
        auto& le = this->les_[mtx_id];
        le.mtx.unlock();
    }
    
private:
    #ifndef MEDSM2_USE_LAD
    struct sig_location {
        proc_id_type    proc;
        byte_rptr_type  sig_rptr;
        size_type       sig_bytes;
    };
    
    sig_location get_sig_location(
        com_itf_type&       com
    ,   const mtx_id_type   mtx_id
    ) {
        const auto num_procs = com.get_num_procs();
        
        const auto proc_id = static_cast<proc_id_type>(mtx_id % num_procs);
        // sig_pos must be smaller than this->num_sigs_.
        const auto sig_pos = mtx_id / num_procs;
        
        return {
            proc_id
        ,   sig_buf_.remote(proc_id, sig_pos * this->sig_bytes_)
        ,   this->sig_bytes_
        };
    }
    #endif
    
    size_type   num_locks_ = 0;
    size_type   sig_bytes_ = 0;
    
    struct local_entry {
        mutex_type  mtx;
    };
    
    mefdn::unique_ptr<local_entry []> les_;
    
    lock_table_type lk_tbl_;
    
    #ifndef MEDSM2_USE_LAD
    typename P::template alltoall_buffer<mefdn::byte>
        sig_buf_;
    #endif
};

} // namespace medsm2
} // namespace menps

