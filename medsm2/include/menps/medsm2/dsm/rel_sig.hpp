
#pragma once

#include <menps/medsm2/common.hpp>
#include <algorithm>

namespace menps {
namespace medsm2 {

template <typename P>
class rel_sig
{
    using blk_id_type = typename P::blk_id_type;
    
    using wn_vector_type = typename P::wn_vector_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    using serialized_buffer_type = typename sig_buffer_type::serialized_buffer_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_guard_type = typename ult_itf_type::template lock_guard<mutex_type>;
    
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
        const mutex_guard_type lk{this->mtx_};
        
        // Merge two sorted lists.
        auto merged_sig = sig_buffer_type::merge(seg_tbl, this->sig_, sig);
        
        merged_sig.truncate(P::constants_type::max_rel_sig_len);
        
        this->sig_ = mefdn::move(merged_sig);
    }
    
    serialized_buffer_type serialize_and_clear(const size_type size) {
        const mutex_guard_type lk{this->mtx_};
        
        auto ret = sig_.serialize(size);
        
        // Initialize the release signature.
        sig_ = sig_buffer_type();
        
        return ret;
    }
    
    static size_type get_max_size_in_bytes() noexcept
    {
        return sig_buffer_type::get_size_in_bytes(P::constants_type::max_rel_sig_len);
    }
    
    sig_buffer_type get_sig(const bool clear_sig)
    {
        const mutex_guard_type lk{this->mtx_};
        
        if (clear_sig) {
            // Move the signature first.
            auto sig = fdn::move(this->sig_);
            // Make sure that the signature is cleared; nothing actually happens.
            this->sig_ = sig_buffer_type();
            return sig;
        }
        else {
            // Copy the signature and return it.
            return this->sig_;
        }
    }
    
private:
    mutable mutex_type  mtx_;
    sig_buffer_type     sig_;
};

} // namespace medsm2
} // namespace menps

