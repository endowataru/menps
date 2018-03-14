
#pragma once

#include <menps/medsm2/common.hpp>
#include <map>
#include <unordered_set>
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
    using mutex_unique_lock_type = typename P::mutex_unique_lock_type;
    
    using size_type = typename P::size_type;
    
public:
    void merge(wn_vector_type wn_vec)
    {
        const mutex_unique_lock_type lk(this->mtx_);
        
        // Sort the write notices.
        auto sig = sig_buffer_type::create_from_wns(mefdn::move(wn_vec));
        
        // Merge two sorted lists.
        auto merged_sig = sig_buffer_type::merge(this->sig_, sig);
        
        merged_sig.truncate(P::constants_type::max_rel_sig_len);
        
        this->sig_ = mefdn::move(merged_sig);
    }
    
    typename sig_buffer_type::serialized_buffer_type serialize(size_type size) {
        // TODO: lock
        const mutex_unique_lock_type lk(this->mtx_);
        
        auto ret = sig_.serialize(size);
        
        // Initialize the release signature.
        sig_ = sig_buffer_type();
        
        return ret;
    }
    
    static size_type get_max_size_in_bytes() noexcept
    {
        return sig_buffer_type::get_size_in_bytes(P::constants_type::max_rel_sig_len);
    }
    
private:
    mutex_type      mtx_;
    sig_buffer_type sig_;
};

} // namespace medsm2
} // namespace menps

