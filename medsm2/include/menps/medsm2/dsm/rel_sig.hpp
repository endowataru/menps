
#pragma once

#include <menps/medsm2/common.hpp>
#include <algorithm>

namespace menps {
namespace medsm2 {

template <typename P>
class rel_sig
{
    using blk_id_type = typename P::blk_id_type;
    
    using wr_ts_type = typename P::wr_ts_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    using size_type = typename P::size_type;
    
public:
    template <typename SegTable>
    void merge(SegTable& seg_tbl, wn_vector_type wn_vec)
    {
        // Sort the write notices.
        auto sig = sig_buffer_type::create_from_wns(mefdn::move(wn_vec));
        
        this->merge(seg_tbl, mefdn::move(sig));
    }
    
    template <typename SegTable>
    void merge(SegTable& seg_tbl, const sig_buffer_type& sig)
    {
        const unique_lock_type lk(this->mtx_);
        
        // Merge two sorted lists.
        auto merged_sig = sig_buffer_type::merge(seg_tbl, this->sig_, sig);
        
        merged_sig.truncate(P::constants_type::max_rel_sig_len);
        
        this->sig_ = mefdn::move(merged_sig);
    }
    
    typename sig_buffer_type::serialized_buffer_type serialize(size_type size) {
        // TODO: lock
        const unique_lock_type lk(this->mtx_);
        
        auto ret = sig_.serialize(size);
        
        // Initialize the release signature.
        sig_ = sig_buffer_type();
        
        return ret;
    }
    
    static size_type get_max_size_in_bytes() noexcept
    {
        return sig_buffer_type::get_size_in_bytes(P::constants_type::max_rel_sig_len);
    }
    
    sig_buffer_type get_sig() const
    {
        const unique_lock_type lk(this->mtx_);
        
        // Copy the signature and return it.
        // TODO: Reduce this copy.
        return this->sig_;
    }
    
private:
    mutable mutex_type  mtx_;
    sig_buffer_type     sig_;
};

} // namespace medsm2
} // namespace menps

