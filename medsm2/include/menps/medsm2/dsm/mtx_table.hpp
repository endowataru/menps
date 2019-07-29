
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
        
        this->num_locks_ = conf.max_num_locks;
        this->sig_bytes_ = conf.sig_size_in_bytes;
        
        this->les_ = mefdn::make_unique<local_entry []>(this->num_locks_);
        
        this->lk_tbl_.coll_make(com, this->num_locks_, this->sig_bytes_);
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
        
        MEFDN_LOG_VERBOSE(
            "msg:Start locking global mutex.\t"
            "mtx_id:0x{:x}"
        ,   mtx_id
        );
        
        MEFDN_ASSERT(P::is_valid_mutex_id(mtx_id));
        
        auto& le = this->les_[mtx_id];
        le.mtx.lock();
        
        auto& p2p = com.get_p2p_lock();
        const auto tag = P::get_tag_from_lock_id(mtx_id);
        
        const auto sig_buf_lptr =
            rma.template make_unique_uninitialized<mefdn::byte []>(this->sig_bytes_);
        
        const auto buf_ptr = sig_buf_lptr.get();
        
        this->lk_tbl_.lock_global(com, p2p, mtx_id, tag, buf_ptr);
        
        // Deserialize the buffer and return it as a signature.
        auto sig_buf = sig_buffer_type::deserialize_from(buf_ptr, this->sig_bytes_);
        
        MEFDN_LOG_VERBOSE(
            "msg:Locked global mutex.\t"
            "mtx_id:0x{:x}"
        ,   mtx_id
        );
        
        return { mefdn::move(sig_buf) };
    }
    
    void unlock(
        com_itf_type&           com
    ,   const mtx_id_type       mtx_id
    ,   const sig_buffer_type&  sig_buf
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Start unlocking global mutex.\t"
            "mtx_id:0x{:x}"
        ,   mtx_id
        );
        
        MEFDN_ASSERT(P::is_valid_mutex_id(mtx_id));
        
        const auto ser_buf = sig_buf.serialize(this->sig_bytes_);
        
        auto& p2p = com.get_p2p_lock();
        const auto tag = P::get_tag_from_lock_id(mtx_id);
        
        this->lk_tbl_.unlock_global(com, p2p, mtx_id, tag, ser_buf.get());
        
        auto& le = this->les_[mtx_id];
        le.mtx.unlock();
        
        MEFDN_LOG_VERBOSE(
            "msg:Unlocked global mutex.\t"
            "mtx_id:0x{:x}"
        ,   mtx_id
        );
    }
    
private:
    size_type   num_locks_ = 0;
    size_type   sig_bytes_ = 0;
    
    struct local_entry {
        mutex_type  mtx;
    };
    
    mefdn::unique_ptr<local_entry []> les_;
    
    lock_table_type lk_tbl_;
};

} // namespace medsm2
} // namespace menps

