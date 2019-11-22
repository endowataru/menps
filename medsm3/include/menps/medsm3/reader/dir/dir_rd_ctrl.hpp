
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_rd_ctrl
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

    using blk_id_type = typename P::blk_id_type;

    using rd_ts_state_type = typename P::rd_ts_state_type;

public:
    explicit dir_rd_ctrl(
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

        CMPTH_P_LOG_INFO(P
        ,   "Start slow read."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        auto rd_ts_st = this->get_rd_ts_st();
        auto up_ret MEFDN_MAYBE_UNUSED = this->update_global(rd_ts_st, blk_llk);

        CMPTH_P_LOG_INFO(P
        ,   "Finish slow read."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "is_write_protected", up_ret.is_write_protected
        ,   "is_written", up_ret.is_written
        );

        return true;
    }

    struct fast_release_result {
        bool            needs_release;
        bool            is_released;
        wn_entry_type   new_wn;
    };

    fast_release_result try_fast_release(const rd_ts_state_type& /*rd_ts_st*/, blk_local_lock_type& /*blk_llk*/)
    {
        return { true, false, wn_entry_type() }; // Note: Use () for GCC 4.8
    }

    void on_start_write(const rd_ts_state_type& /*rd_ts_st*/, blk_local_lock_type& /*blk_llk*/) {
        // This function is called for fast release in ts_rd_ctrl.
    }

    struct update_result {
        // Indicate that this block can be removed from the write set.
        bool    is_write_protected;
        // 
        bool    is_written;
        
        wn_entry_type new_wn;
    };

    update_result update_global(const rd_ts_state_type& /*rd_ts_st*/, blk_local_lock_type& blk_llk)
    {
        CMPTH_P_LOG_INFO(P
        ,   "Start updating block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        const auto is_upgraded = this->state_data_ctrl().is_invalid(blk_llk);
        
        auto blk_glk = this->home_ctrl().get_global_lock(blk_llk);
        //{ owner_proc, owner_intvl } = home_ctrl().lock_global(blk_cs);

        const bool is_remotely_updated_ts = true;
        auto sd_ret = this->state_data_ctrl().update_global(blk_glk, is_remotely_updated_ts);
        
        // update timestamps
        auto new_sh_map = this->inv_ctrl().send_inv_to_sharers(blk_glk, sd_ret);

        if (is_upgraded) {
            this->rd_set_.add_readable(blk_llk.blk_id()/*, inv_intvl.rd_ts*/);
        }
        
        //home_ctrl().unlock_global(blk_cs, intvl);
        blk_glk.unlock({ sd_ret.new_last_writer_proc, fdn::move(new_sh_map) });

        CMPTH_P_LOG_INFO(P
        ,   "Finish updating block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "is_upgraded", is_upgraded
        );
        
        return { sd_ret.is_write_protected, sd_ret.is_written, wn_entry_type() };
    }

    static std::string show_wn(const wn_entry_type& /*wn*/) {
        return "none";
    }

    void fence_acquire_all(const sig_buf_set_type& /*sig_set*/) {
        this->fence_acquire();
    }
    void fence_acquire(const sig_buffer_type& /*sig*/) {
        this->fence_acquire();
    }

private:
    struct self_invalidate_result {
        // This block will be still readable after this invalidation.
        bool still_readable;
    };

public:
    void fence_acquire()
    {
        CMPTH_P_LOG_INFO(P, "Start fence acquire.");

        this->rd_set_.self_invalidate(
            [this] (const rd_ts_state_type& /*rd_ts_st*/, const blk_id_type blk_id) -> self_invalidate_result {
                auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
                const auto inv_ctrl_ret = this->inv_ctrl().try_invalidate(blk_llk);
                if (inv_ctrl_ret.needs_invalidate) {
                    this->invalidate(blk_llk);
                }

                const auto still_readable =
                    ! this->state_data_ctrl().is_invalid(blk_llk);
                return { still_readable };
            }
        );

        CMPTH_P_LOG_INFO(P, "Finish fence acquire.");
    }

private:
    void invalidate(blk_local_lock_type& blk_llk)
    {
        CMPTH_P_LOG_INFO(P
        ,   "Invalidate block."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        auto inv_ret = this->state_data_ctrl().invalidate(blk_llk);
        if (inv_ret.needs_merge) {
            const auto rd_ts_st = this->get_rd_ts_st();
            const auto up_ret MEFDN_MAYBE_UNUSED =
                this->update_global(rd_ts_st, blk_llk);
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

