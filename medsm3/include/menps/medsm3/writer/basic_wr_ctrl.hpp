
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_wr_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using rd_ctrl_type = typename P::rd_ctrl_type;
    using rd_ctrl_ptr_type = typename P::rd_ctrl_ptr_type;
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using rd_ts_state_type = typename P::rd_ts_state_type;
    using sig_buffer_type = typename P::sig_buffer_type;
    using rel_sig_type = typename P::rel_sig_type;
    using wn_vector_type = typename P::wn_vector_type;
    using wn_entry_type = typename P::wn_entry_type;
    using wr_set_type = typename P::wr_set_type;

    using blk_id_type = typename P::blk_id_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using ult_itf_type = typename P::ult_itf_type;

public:
    explicit basic_wr_ctrl(rd_ctrl_ptr_type rd_ctrl_ptr)
        : rd_ctrl_ptr_{fdn::move(rd_ctrl_ptr)}
    { }

    MEFDN_NODISCARD
    bool try_start_write(blk_local_lock_type& blk_llk)
    {
        auto& sd_ctrl = this->rd_ctrl().state_data_ctrl();

        const auto sd_ret = sd_ctrl.start_write(blk_llk);
        if (sd_ret.needs_protect) {
            this->wr_set_.add_writable(blk_llk.blk_id());

            auto rd_ts_st = this->rd_ctrl().get_rd_ts_st();
            this->rd_ctrl().on_start_write(rd_ts_st, blk_llk);
        }

        // When the block was or is now writable, consider this method call succeeded.
        // Otherwise, consider that this method call failed.
        return sd_ret.is_writable;
    }

    sig_buffer_type fence_release()
    {
        CMPTH_P_LOG_INFO(P, "msg:Start release fence.", 0);

        auto& self = this->derived();

        auto rd_ts_st = this->rd_ctrl().get_rd_ts_st();

        if (self.is_fast_release_enabled()) {
            // Before loading the link values of blk_lock_table,
            // it is necessary to complete all the writes in this process.
            ult_itf_type::atomic_thread_fence(ult_itf_type::memory_order_seq_cst);
        }
        
        // Iterate all of the writable blocks.
        // If the callback returns false,
        // the corresponding block will be removed in the next release.
        auto wrs_ret =
            this->wr_set_.start_release_for_all_blocks(
                rd_ts_st
            ,   [this] (const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id) {
                    return this->release(rd_ts_st, blk_id);
                }
            ,   convert_to_wn_vec{self}
            );
        
        if (wrs_ret.needs_release) {
            //#ifndef MEDSM2_USE_DIRECTORY_COHERENCE
            if (self.is_signature_enabled()) {
                // Union the write notice vector and the release signature.
                auto flag_set_ref = self.get_flag_set_ref();
                this->rel_sig_.merge(flag_set_ref, fdn::move(wrs_ret.wn_vec));
            }
            
            // Notify the other threads waiting for the finish of the release fence.
            this->wr_set_.finish_release();
        }

        CMPTH_P_LOG_INFO(P, "msg:Finish release fence.", 0);
        
        return this->rel_sig_.get_sig();
    }

private:
    struct convert_to_wn_vec
    {
        derived_type&   self;
        
        template <typename BlkIdIter, typename RelRetIter>
        wn_vector_type operator() (
            BlkIdIter   blk_id_first
        ,   BlkIdIter   blk_id_last
        ,   RelRetIter  rel_ret_first
        ) {
            const auto num_released =
                static_cast<fdn::size_t>(blk_id_last - blk_id_first);

            wn_vector_type wn_vec;
            // Pre-allocate the write notice vector.
            wn_vec.reserve(num_released);
            
            // Check all of the release results sequentially.
            for (fdn::size_t i = 0; i < num_released; ++i) {
                const auto& rel_ret = *(rel_ret_first+i);
                if (rel_ret.release_completed && rel_ret.is_written) {
                    // Add to the write notices
                    // because the current process modified this block.
                    wn_vec.push_back(rel_ret.wn);
                }
            }
            return wn_vec;
        }
    };

    struct release_result {
        bool            release_completed;
        bool            is_written;
        // Indicate that this block can be removed from the write set.
        bool            is_still_writable;
        wn_entry_type   wn;
    };

    release_result release(const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id)
    {
        auto& ll_ctrl = this->rd_ctrl().local_lock_ctrl();

        auto blk_llk = ll_ctrl.get_local_lock(blk_id);
        
        auto fast_rel_ret = this->rd_ctrl().try_fast_release(rd_ts_st, blk_llk);
        if (!fast_rel_ret.needs_release) {
            return { false, false, false, {} };
        }
        else if (fast_rel_ret.is_released) {
            CMPTH_P_LOG_INFO(P,
                "Fast release."
            ,   4
            ,   "blk_id", blk_id.to_str()
            ,   "home_proc", fast_rel_ret.new_wn.home_proc
            ,   "wr_ts", fast_rel_ret.new_wn.ts_intvl.wr_ts
            ,   "rd_ts", fast_rel_ret.new_wn.ts_intvl.rd_ts
            );
            return { true, true, true, fast_rel_ret.new_wn };
        }
        else {
            CMPTH_P_LOG_INFO(P,
                "Start slow release."
            ,   1
            ,   "blk_id", blk_id.to_str()
            );

            auto up_ret = this->rd_ctrl().update_global(rd_ts_st, blk_llk);

            CMPTH_P_LOG_INFO(P,
                "Finish slow release."
            ,   6
            ,   "blk_id", blk_id.to_str()
            ,   "home_proc", up_ret.new_wn.home_proc
            ,   "wr_ts", up_ret.new_wn.ts_intvl.wr_ts
            ,   "rd_ts", up_ret.new_wn.ts_intvl.rd_ts
            ,   "is_write_protected", up_ret.is_write_protected
            ,   "is_written", up_ret.is_written
            );
            return { true, up_ret.is_written, !up_ret.is_write_protected, up_ret.new_wn };
        }
    }

public:
    rd_ctrl_type& rd_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->rd_ctrl_ptr_);
        return *this->rd_ctrl_ptr_;
    }

private:
    rd_ctrl_ptr_type    rd_ctrl_ptr_;
    wr_set_type         wr_set_;
    rel_sig_type        rel_sig_;
};

} // namespace medsm3
} // namespace menps

