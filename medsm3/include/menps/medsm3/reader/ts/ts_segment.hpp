
#pragma once

#include <menps/medsm3/layout/basic_segment.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_segment
    : public basic_segment<P>
{
    using base = basic_segment<P>;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using ts_interval_type = typename P::ts_interval_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_id_type = typename P::blk_id_type;

    using wr_count_type = typename P::wr_count_type;
    
public:
    explicit ts_segment(com_itf_type& com, const fdn::size_t num_blks)
        : base{com, num_blks, sizeof(global_entry_type)}
        , lk_entries_{fdn::make_unique<local_entry_type []>(num_blks)}
    {
        const auto this_proc = com.this_proc_id();
        for (fdn::size_t sidx = 0; sidx < num_blks; ++sidx) {
            // Set this process as the "first home".
            // This means that every reader starts with its data first.
            // TODO: This is a bit weird
            //       because "home" is not the current process in general.
            this->lk_entries_[sidx].wn_proc = this_proc;
            
            this->lk_entries_[sidx].fast_rel_threshold = 1;
        }
    }

    struct global_entry_type
    {
        proc_id_type        last_writer_proc;
        ts_interval_type    owner_intvl;
    };

    struct local_entry_type
        : base::blk_local_metadata_base
    {
        // The number of transactions without writing the data.
        wr_count_type   wr_count;

        // TODO: These members should only appear when fast release is enabled.
        bool            is_written_last;
        wr_count_type   fast_rel_count;
        wr_count_type   fast_rel_threshold;

        // originally blk_dir_table.le.home_proc
        proc_id_type        wn_proc;
        // originally blk_dir_table.le.cur_wr_ts / cur_rd_ts
        ts_interval_type    wn_intvl;
    };
 
    local_entry_type& get_local_entry(blk_local_lock_type& blk_llk) {
        return this->get_local_entry_of_blk_id(blk_llk.blk_id());
    }   
    local_entry_type& get_local_entry_of_blk_id(const blk_id_type blk_id) {
        CMPTH_P_ASSERT(P, blk_id.sidx < this->get_num_blks());
        return this->lk_entries_[blk_id.sidx];
    }   
    
private:
    fdn::unique_ptr<local_entry_type []> lk_entries_;
};

} // namespace medsm3
} // namespace menps

