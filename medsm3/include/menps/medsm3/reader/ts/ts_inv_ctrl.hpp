
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_inv_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_type = typename P::blk_global_lock_type;

    using ts_type = typename P::ts_type;
    using ts_interval_type = typename P::ts_interval_type;
    using wn_entry_type = typename P::wn_entry_type;

    using rd_ts_state_type = typename P::rd_ts_state_type;

public:
    bool try_get_wn(rd_ts_state_type& rd_ts_st, blk_local_lock_type& blk_llk, wn_entry_type* const out_wn)
    {
        auto& self = this->derived();
        const auto local_wn = self.get_local_wn(blk_llk);
        const auto min_wr_ts = rd_ts_st.get_min_wr_ts();
        if (min_wr_ts <= local_wn.ts_intvl.rd_ts) {
            *out_wn = local_wn;
            return true;
        }
        else {
            return false;
        }
    }

    bool is_remotely_updated_ts(blk_global_lock_type& blk_glk)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();

        const auto cur_wn = self.get_local_wn(blk_llk);
        const auto home_intvl = blk_glk.get_home_interval();

        return cur_wn.ts_intvl.wr_ts < home_intvl.wr_ts;
    }

    ts_interval_type fast_release(
        const rd_ts_state_type&     rd_ts_st
    ,   blk_local_lock_type&        blk_llk
    ,   const ts_interval_type      intvl
    ) {
        auto& self = this->derived();
        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();
        
        self.set_local_wn(blk_llk, this_proc, intvl);
        
        const auto new_intvl = this->make_new_ts(rd_ts_st, true, intvl.wr_ts, intvl.rd_ts);
        return new_intvl;
    }

    ts_interval_type on_start_write(
        const rd_ts_state_type&     rd_ts_st
    ,   blk_local_lock_type&        /*blk_llk*/
    ,   const ts_interval_type      intvl
    ) {
        return this->make_new_ts(rd_ts_st, true, intvl.wr_ts, intvl.rd_ts);
    }

    template <typename UpdateResult>
    ts_interval_type update_timestamp(
        const rd_ts_state_type& rd_ts_st
    ,   blk_global_lock_type&   blk_glk
    ,   const UpdateResult&     up_ret
    ,   const bool              is_upgraded
    ) {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();
        auto copied_ts_st = rd_ts_st;
        
        //if (old_state == state_type::invalid_dirty || old_state == state_type::invalid_clean)
        if (is_upgraded) {
            auto& rd_set = copied_ts_st.get_rd_set();
            // Reload the timestamp state.
            copied_ts_st = rd_set.get_ts_state();
        }

        const auto old_intvl = blk_glk.get_home_interval();

        // Generate new timestamp values.
        const auto new_intvl =
            this->make_new_ts(copied_ts_st, up_ret.is_written,
                old_intvl.wr_ts, old_intvl.rd_ts);
        // update timestamp based on blk_gcs.ts
        
        // Update the timestamps because this process lastly released.
        self.set_local_wn(blk_llk, up_ret.new_owner, new_intvl);

        return new_intvl;
    }

    struct try_invalidate_with_ts_result {
        bool    needs_invalidate;
        ts_type rd_ts;
    };

    try_invalidate_with_ts_result try_invalidate_with_ts(blk_local_lock_type& blk_llk, const rd_ts_state_type& rd_ts_st)
    {
        auto& self = this->derived();
        const auto local_wn = self.get_local_wn(blk_llk);
        const auto invalidated = P::is_greater_ts(rd_ts_st.get_min_wr_ts(), local_wn.ts_intvl.rd_ts);
        return { invalidated, local_wn.ts_intvl.rd_ts };
    }

    bool try_invalidate_with_wn(blk_local_lock_type& blk_llk, const wn_entry_type& new_wn)
    {
        auto& self = this->derived();
        const auto local_wn = self.get_local_wn(blk_llk);
        const auto invalidated = P::is_greater_ts(new_wn.ts_intvl.wr_ts, local_wn.ts_intvl.rd_ts);
        
        if (invalidated) {
            self.set_local_wn(blk_llk, new_wn.home_proc, new_wn.ts_intvl);
        }

        return invalidated;
    }

private:
    ts_interval_type make_new_ts(
        const rd_ts_state_type& rd_ts_st
    ,   const bool              is_written
    ,   const ts_type           wr_ts
    ,   const ts_type           rd_ts
    ) {
        // If the data is written, the timestamp is updated.
        // make_new_wr_ts() calculates new_wr_ts = max(old_rd_ts+1, acq_ts).
        // If not, because the data is read from the old owner
        // (that will be the owner again),
        // the timestamp becomes equal to that of the owner.
        const auto new_wr_ts =
            is_written ? rd_ts_st.make_new_wr_ts(rd_ts) : wr_ts;
        
        // The read timestamp depends on the write timestamp.
        // This calculates new_rd_ts = max(new_wr_ts+lease, old_rd_ts).
        // TODO: Provide a good prediction for a lease value of each block.
        const auto new_rd_ts =
            rd_ts_st.make_new_rd_ts(new_wr_ts, rd_ts);
        
        return { new_wr_ts, new_rd_ts };
    }
};

} // namespace medsm3
} // namespace menps

