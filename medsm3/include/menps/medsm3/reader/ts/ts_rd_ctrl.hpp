
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_rd_ctrl
{
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using local_lock_ctrl_type = typename P::local_lock_ctrl_type;
    using local_lock_ctrl_ptr_type = typename P::local_lock_ctrl_ptr_type;
    using home_ctrl_type = typename P::home_ctrl_type;
    using home_ctrl_ptr_type = typename P::home_ctrl_ptr_type;
    using state_data_ctrl_type = typename P::state_data_ctrl_type;
    using state_data_ctrl_ptr_type = typename P::state_data_ctrl_ptr_type;
    using inv_ctrl_type = typename P::inv_ctrl_type;
    using inv_ctrl_ptr_type = typename P::inv_ctrl_ptr_type;
    using rd_set_type = typename P::rd_set_type;

    using sig_buffer_type = typename P::sig_buffer_type;
    using sig_buf_set_type = typename P::sig_buf_set_type;
    using wn_entry_type = typename P::wn_entry_type;
    using wr_ts_type = typename P::wr_ts_type;
    using rd_ts_type = typename P::rd_ts_type;

    using blk_id_type = typename P::blk_id_type;

    using rd_ts_state_type = typename P::rd_ts_state_type;

public:
    explicit ts_rd_ctrl(
        local_lock_ctrl_ptr_type    ll_ctrl_ptr
    ,   home_ctrl_ptr_type          home_ctrl_ptr
    ,   state_data_ctrl_ptr_type    sd_ctrl_ptr
    ,   inv_ctrl_ptr_type           inv_ctrl_ptr
    )
        : ll_ctrl_ptr_{fdn::move(ll_ctrl_ptr)}
        , home_ctrl_ptr_{fdn::move(home_ctrl_ptr)}
        , sd_ctrl_ptr_{fdn::move(sd_ctrl_ptr)}
        , inv_ctrl_ptr_{fdn::move(inv_ctrl_ptr)}
    { }

    MEFDN_NODISCARD
    bool try_start_read(blk_local_lock_type& blk_llk)
    {
        if (!this->state_data_ctrl().is_invalid(blk_llk)) {
            return false;
        }

        auto rd_ts_st = this->get_rd_ts_st();
        wn_entry_type wn = wn_entry_type();
        if (this->inv_ctrl().try_get_wn(rd_ts_st, blk_llk, &wn)) {
            CMPTH_P_LOG_INFO(P
            ,   "Start fast read."
            ,   "blk_id", blk_llk.blk_id().to_str()
            ,   "home_proc", wn.home_proc
            ,   "wr_ts", wn.ts_intvl.wr_ts
            ,   "rd_ts", wn.ts_intvl.rd_ts
            );

            this->state_data_ctrl().fast_read(blk_llk, wn.home_proc);
            // Add this block to the read set.
            this->rd_set_.add_readable(blk_llk.blk_id(), wn.ts_intvl.rd_ts);

            CMPTH_P_LOG_INFO(P
            ,   "Finish fast read."
            ,   "blk_id", blk_llk.blk_id().to_str()
            ,   "home_proc", wn.home_proc
            ,   "wr_ts", wn.ts_intvl.wr_ts
            ,   "rd_ts", wn.ts_intvl.rd_ts
            );
        }
        else {
            CMPTH_P_LOG_INFO(P
            ,   "Start slow read."
            ,   "blk_id", blk_llk.blk_id().to_str()
            );

            auto up_ret MEFDN_MAYBE_UNUSED = this->update_global(rd_ts_st, blk_llk);

            CMPTH_P_LOG_INFO(P
            ,   "Finish slow read."
            ,   "blk_id", blk_llk.blk_id().to_str()
            ,   "home_proc", up_ret.new_wn.home_proc
            ,   "wr_ts", up_ret.new_wn.ts_intvl.wr_ts
            ,   "rd_ts", up_ret.new_wn.ts_intvl.rd_ts
            ,   "is_write_protected", up_ret.is_write_protected
            ,   "is_written", up_ret.is_written
            );
        }
        return true;
    }

    struct fast_release_result {
        bool            needs_release;
        bool            is_released;
        wn_entry_type   new_wn;
    };

    template <typename R = fast_release_result>
    fdn::enable_if_t<P::constants_type::is_fast_release_enabled, R>
    try_fast_release(const rd_ts_state_type& rd_ts_st, blk_local_lock_type& blk_llk)
    {
        const auto chk_rel_ret = this->state_data_ctrl().check_release(blk_llk);
        if (!chk_rel_ret.needs_release) {
            return { false, false, wn_entry_type() }; // Note: Use () for GCC 4.8
        }

        if (!(chk_rel_ret.is_fast_released && this->home_ctrl().check_owned(blk_llk))) {
            return { true, false, wn_entry_type() }; // Note: Use () for GCC 4.8
        }

        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();

        auto old_ge = this->home_ctrl().read_lock_entry(blk_llk);
        CMPTH_P_ASSERT(P, old_ge.last_writer_proc == this_proc);

        auto new_intvl = this->inv_ctrl().fast_release(rd_ts_st, blk_llk, old_ge.owner_intvl);

        this->home_ctrl().write_lock_entry(blk_llk, { this_proc, new_intvl });

        return { true, true, { this_proc, blk_llk.blk_id(), old_ge.owner_intvl } };
    }

    template <typename R = fast_release_result>
    fdn::enable_if_t<!P::constants_type::is_fast_release_enabled, R>
    try_fast_release(const rd_ts_state_type& /*rd_ts_st*/, blk_local_lock_type& /*blk_llk*/) {
        // This is only enabled when fast release is enabled.
        return { true, false, wn_entry_type() }; // Note: Use () for GCC 4.8
    }

    template <typename R = void>
    fdn::enable_if_t<P::constants_type::is_fast_release_enabled, R>
    on_start_write(const rd_ts_state_type& rd_ts_st, blk_local_lock_type& blk_llk)
    {
        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();

        if (this->state_data_ctrl().is_written_last(blk_llk)) {
            auto old_ge = this->home_ctrl().read_lock_entry(blk_llk);
            CMPTH_P_ASSERT(P, old_ge.last_writer_proc == this_proc);

            auto new_intvl = this->inv_ctrl().on_start_write(rd_ts_st, blk_llk, old_ge.owner_intvl);

            this->home_ctrl().write_lock_entry(blk_llk, { this_proc, new_intvl });
        }
    }

    template <typename R = void>
    fdn::enable_if_t<!P::constants_type::is_fast_release_enabled, R>
    on_start_write(const rd_ts_state_type& /*rd_ts_st*/, blk_local_lock_type& /*blk_llk*/) {
        // This is only enabled when fast release is enabled.
    }

    struct update_result {
        // Indicate that this block can be removed from the write set.
        bool    is_write_protected;
        // 
        bool    is_written;
        
        wn_entry_type new_wn;
    };

    update_result update_global(const rd_ts_state_type& rd_ts_st, blk_local_lock_type& blk_llk)
    {
        CMPTH_P_LOG_INFO(P
        ,   "Start updating block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        const auto is_upgraded = this->state_data_ctrl().is_invalid(blk_llk);
        
        auto blk_glk = this->home_ctrl().get_global_lock(blk_llk);
        //{ owner_proc, owner_intvl } = home_ctrl().lock_global(blk_cs);

        const bool is_remotely_updated_ts =
            this->inv_ctrl().is_remotely_updated_ts(blk_glk);

        auto sd_ret = this->state_data_ctrl().update_global(blk_glk, is_remotely_updated_ts);
        
        // update timestamps
        auto inv_intvl = this->inv_ctrl().update_timestamp(rd_ts_st, blk_glk, sd_ret, is_upgraded);

        if (is_upgraded) {
            this->rd_set_.add_readable(blk_llk.blk_id(), inv_intvl.rd_ts);
        }
        
        //home_ctrl().unlock_global(blk_cs, intvl);
        blk_glk.unlock({ sd_ret.new_last_writer_proc, inv_intvl });

        CMPTH_P_LOG_INFO(P
        ,   "Finish updating block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "is_upgraded", is_upgraded
        );
        
        return { sd_ret.is_write_protected, sd_ret.is_written,
            { sd_ret.new_owner, blk_llk.blk_id(), inv_intvl } };
    }

    static std::string show_wn(const wn_entry_type& wn)
    {
        return fmt::format(
            "home_proc={},wr_ts={},rd_ts={}"
        ,   wn.home_proc
        ,   wn.ts_intvl.wr_ts
        ,   wn.ts_intvl.rd_ts
        );
    }

    void fence_acquire_all(const sig_buf_set_type& sig_set)
    {
        for (auto& sig : sig_set) {
            this->fence_acquire(sig);
        }
    }

    void fence_acquire(const sig_buffer_type& sig)
    {
        CMPTH_P_LOG_INFO(P, "Start fence acquire.");

        this->rd_set_.self_invalidate(
            sig.get_min_wr_ts()
        ,   [this] (const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id) -> invalidate_result {
                auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
                const auto inv_ctrl_ret = this->inv_ctrl().try_invalidate_with_ts(blk_llk, rd_ts_st);
                if (inv_ctrl_ret.needs_invalidate) {
                    auto up_ret = this->invalidate(blk_llk);
                    const auto rd_ts = up_ret.is_updated ? up_ret.rd_ts : inv_ctrl_ret.rd_ts;
                    return { up_ret.is_updated, rd_ts };
                }
                else {
                    return { false, inv_ctrl_ret.rd_ts };
                }
            }
        );

        sig.for_all_wns(
            [this] (const wn_entry_type& wn) {
                auto blk_llk = this->local_lock_ctrl().get_local_lock(wn.blk_id);
                if (this->inv_ctrl().try_invalidate_with_wn(blk_llk, wn)) {
                    this->invalidate(blk_llk);
                }
            }
        );

        CMPTH_P_LOG_INFO(P, "Finish fence acquire.");
    }

private:
    struct invalidate_result {
        bool        is_updated;
        rd_ts_type  rd_ts;
    };
    
    invalidate_result invalidate(blk_local_lock_type& blk_llk)
    {
        CMPTH_P_LOG_INFO(P
        ,   "Invalidate block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        auto inv_ret = this->state_data_ctrl().invalidate(blk_llk);
        if (inv_ret.needs_merge) {
            const auto rd_ts_st = this->get_rd_ts_st();
            const auto up_ret = this->update_global(rd_ts_st, blk_llk);
            // FIXME: up_ret.new_wn is lost from rel_sig ???
            return { true, up_ret.new_wn.ts_intvl.rd_ts };
        }
        else {
            return { false, rd_ts_type() }; // Note: Use () for GCC 4.8
        }
    }

public:
    local_lock_ctrl_type& local_lock_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->ll_ctrl_ptr_);
        return *this->ll_ctrl_ptr_;
    }
    state_data_ctrl_type& state_data_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->sd_ctrl_ptr_);
        return *this->sd_ctrl_ptr_;
    }

    rd_ts_state_type get_rd_ts_st() { return this->rd_set_.get_ts_state(); }

private:
    home_ctrl_type& home_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->home_ctrl_ptr_);
        return *this->home_ctrl_ptr_;
    }
    inv_ctrl_type& inv_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->inv_ctrl_ptr_);
        return *this->inv_ctrl_ptr_;
    }

    local_lock_ctrl_ptr_type    ll_ctrl_ptr_;
    home_ctrl_ptr_type          home_ctrl_ptr_;
    state_data_ctrl_ptr_type    sd_ctrl_ptr_;
    inv_ctrl_ptr_type           inv_ctrl_ptr_;
    rd_set_type                 rd_set_;
};

} // namespace medsm3
} // namespace menps

