
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/assert.hpp>
#include <algorithm>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class sig_buffer
{
    using wn_entry_type = typename P::wn_entry_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using wr_ts_type = typename P::wr_ts_type;
    using blk_id_type = typename P::blk_id_type;
    
    using size_type = typename P::size_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    
    struct header {
        wr_ts_type  min_wr_ts;
        size_type   num_entries;
    };
    
    struct greater_wn {
        bool operator() (const wn_entry_type& a, const wn_entry_type& b) const noexcept {
            return P::is_greater_wr_ts(a.wr_ts, b.wr_ts);
        }
    };
    
public:
    sig_buffer() = default;
    
    /*implicit*/ sig_buffer(wr_ts_type min_wr_ts, wn_vector_type wn_vec)
        : min_wr_ts_(min_wr_ts)
        , wn_vec_(mefdn::move(wn_vec))
    { }
    
    static sig_buffer create_from_wns(wn_vector_type wn_vec)
    {
        // Sort the write notices by their write timestamps.
        std::sort(wn_vec.begin(), wn_vec.end(), greater_wn{});
        
        return sig_buffer(P::make_init_wr_ts(), mefdn::move(wn_vec));
    }
    
    static sig_buffer create_from_ts(const wr_ts_type min_wr_ts)
    {
        return sig_buffer(min_wr_ts, wn_vector_type{});
    }
    
    void truncate(const size_type max_size)
    {
        if (this->wn_vec_.size() > max_size) {
            const auto last_wr_ts = this->wn_vec_[max_size].wr_ts;
            // TODO: Use the policy's comparator.
            this->min_wr_ts_ = std::max(this->min_wr_ts_, last_wr_ts);
            
            this->wn_vec_.erase(this->wn_vec_.begin() + max_size, this->wn_vec_.end());
        }
    }
    
    // Merge two signature and return a merged signature.
    template <typename SegTable> // TODO
    static sig_buffer merge(SegTable& seg_tbl,
        const sig_buffer& sig_a, const sig_buffer& sig_b)
    {
        wn_vector_type new_wn_vec;
        // Pre-allocate the merged vector.
        new_wn_vec.reserve(sig_a.wn_vec_.size() + sig_b.wn_vec_.size());
        
        #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
        auto sig_a_itr = sig_a.wn_vec_.begin();
        auto sig_b_itr = sig_b.wn_vec_.begin();
        const auto sig_a_last = sig_a.wn_vec_.end();
        const auto sig_b_last = sig_b.wn_vec_.end();
        
        {
            const auto lk = seg_tbl.get_flags_lock();
            
            while (sig_a_itr != sig_a_last || sig_b_itr != sig_b_last)
            {
                bool use_a = false;
                if (sig_a_itr != sig_a_last && sig_b_itr != sig_b_last) {
                    use_a = greater_wn{}(*sig_a_itr, *sig_b_itr);
                }
                else {
                    use_a = sig_a_itr != sig_a_last;
                }
                
                const auto& wn = use_a ? *sig_a_itr : *sig_b_itr;
                const auto blk_id = wn.blk_id;
                
                if (seg_tbl.try_set_flag(blk_id)) {
                    // Add to new_wn_vec.
                    new_wn_vec.push_back(wn);
                }
                else {
                    // Ignore.
                }
                
                if (use_a)  ++sig_a_itr;
                else        ++sig_b_itr;
            }
            
            for (const auto& wn : new_wn_vec) {
                seg_tbl.unset_flag(wn.blk_id);
            }
        }
        
        
        #else
        // Merge two sorted lists of write notices based on their write timestamps.
        std::merge(
            sig_a.wn_vec_.begin(), sig_a.wn_vec_.end(),
            sig_b.wn_vec_.begin(), sig_b.wn_vec_.end(),
            std::back_inserter(new_wn_vec),
            greater_wn{});
        
        // Remove duplicated entries from the write notices based on their block IDs.
        // TODO: Do this process inside the merge above.
        {
            std::unordered_set<blk_id_type> exist_ids;
            auto nwv_result = new_wn_vec.begin();
            const auto nwv_last = new_wn_vec.end();
            
            for (auto nwv_itr = new_wn_vec.begin(); nwv_itr != nwv_last; ++nwv_itr)
            {
                const auto blk_id = nwv_itr->blk_id;
                
                if (exist_ids.find(blk_id) == exist_ids.end()) {
                    // A new block ID was found.
                    exist_ids.insert(blk_id);
                    
                    // See also the implementation of std::remove_if().
                    if (nwv_result != nwv_itr) {
                        *nwv_result = std::move(*nwv_itr);
                    }
                    ++nwv_result;
                }
                else {
                    // The same block ID was updated.
                }
            }
            
            // Remove the duplicated entries.
            new_wn_vec.erase(nwv_result, nwv_last);
        }
        #endif
        
        // TODO: Use the policy's comparator.
        const auto new_min_wr_ts = std::max(sig_a.min_wr_ts_, sig_b.min_wr_ts_);
        
        return sig_buffer(new_min_wr_ts, mefdn::move(new_wn_vec));
    }
    
    void serialize_to(void* const dest, const size_type dest_size) const
    {
        const size_type required =
            sizeof(header) + this->wn_vec_.size() * sizeof(wn_entry_type);
        
        MEFDN_ASSERT(dest_size >= required);
    
        const auto h = static_cast<header*>(dest);
        h->min_wr_ts = this->min_wr_ts_;
        h->num_entries = this->wn_vec_.size();
        
        const auto wrs = reinterpret_cast<wn_entry_type*>(h+1);
        std::copy(this->wn_vec_.begin(), this->wn_vec_.end(), wrs);
    }
    
    static constexpr size_type get_size_in_bytes(const size_type num_wns) noexcept {
        return sizeof(header) + num_wns * sizeof(wn_entry_type);
    }
    
    using serialized_buffer_type = mefdn::unique_ptr<mefdn::byte []>;
    
    serialized_buffer_type serialize(const size_type size) const
    {
        const size_type required =
            get_size_in_bytes(this->wn_vec_.size());
        
        MEFDN_ASSERT(required <= size);
        
        // Avoid initializing the array with zero.
        // TODO: Make a special "make_unique" function that doesn't value-initialize the elements.
        serialized_buffer_type buf(new mefdn::byte[size]);
        
        this->serialize_to(buf.get(), size);
        
        return buf;
    }
    
    static sig_buffer deserialize_from(const void* src, const size_type src_size)
    {
        MEFDN_ASSERT(src_size >= sizeof(header));
        
        const auto h = static_cast<const header*>(src);
        const auto min_wr_ts = h->min_wr_ts;
        const auto num_entries = h->num_entries;
        
        const size_type required =
            get_size_in_bytes(h->num_entries);
        
        MEFDN_ASSERT(src_size >= required);
        
        const auto wrs = reinterpret_cast<const wn_entry_type*>(h+1);
        wn_vector_type wn_vec(wrs, wrs + num_entries);
        
        return sig_buffer(min_wr_ts, mefdn::move(wn_vec));
    }
    
    // getter member functions
    
    wr_ts_type get_min_wr_ts() const noexcept {
        return this->min_wr_ts_;
    }
    
    template <typename Func>
    void for_all_wns(Func func) const
    {
        //for (auto& wn : wn_vec_) {
        ult_itf_type::for_loop(
            ult_itf_type::execution::seq
            //ult_itf_type::execution::par
            // TODO: This can be parallelized, but tasks are fine-grained
        ,   0
        ,   this->wn_vec_.size()
        ,   [this, &func] (const size_type i) {
                const auto wn = this->wn_vec_[i];
                func(wn);
            }
        );
    }
    
private:
    wr_ts_type      min_wr_ts_ = 0;
    wn_vector_type  wn_vec_;
};

} // namespace medsm2
} // namespace menps

