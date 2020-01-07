
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_state_data_ctrl
{
    using state_ctrl_type = typename P::state_ctrl_type;
    using state_ctrl_ptr_type = typename P::state_ctrl_ptr_type;
    using data_ctrl_type = typename P::data_ctrl_type;
    using data_ctrl_ptr_type = typename P::data_ctrl_ptr_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_base_type = typename P::blk_global_lock_base_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    
public:
    explicit basic_state_data_ctrl(
        state_ctrl_ptr_type st_ctrl_ptr
    ,   data_ctrl_ptr_type  dt_ctrl_ptr
    )
        : st_ctrl_ptr_{fdn::move(st_ctrl_ptr)}
        , dt_ctrl_ptr_{fdn::move(dt_ctrl_ptr)}
    { }

    bool is_invalid(blk_local_lock_type& blk_llk) {
        return this->st_ctrl().is_invalid(blk_llk);
    }
    bool is_written_last(blk_local_lock_type& blk_llk) {
        return this->st_ctrl().is_written_last(blk_llk);
    }

    void fast_read(blk_local_lock_type& blk_llk, const proc_id_type proc_id)
    {
        // 
        const auto st_ret = this->st_ctrl().fast_read(blk_llk);
        // 
        this->dt_ctrl().fast_read(blk_llk, proc_id, st_ret.is_dirty);
    }

    typename state_ctrl_type::start_write_result start_write(blk_local_lock_type& blk_llk)
    {
        auto st_ret = this->st_ctrl().start_write(blk_llk);
        if (st_ret.needs_protect) {
            this->dt_ctrl().start_write(blk_llk, st_ret.needs_twin);
        }
        return st_ret;
    }

    typename state_ctrl_type::check_release_result check_release(blk_local_lock_type& blk_llk)
    {
        return this->st_ctrl().check_release(blk_llk);
    }

    struct update_result
    {
        // Indicate that this block can be removed from the write set.
        bool            is_write_protected;
        // Indicate that this block is modified by this process.
        bool            is_written;
        // The owner (?) process, which will be written in write notices.
        // This process is used for being read by other reader processes.
        proc_id_type    new_owner;
        // The process which wrote the block at last.
        // This is different from the owner because the owner does not change
        // when the fixed-home mode is selected.
        proc_id_type    new_last_writer_proc;
    };

    update_result update_global(blk_global_lock_base_type& blk_glk, const bool is_remotely_updated_ts)
    {
        auto& blk_llk = blk_glk.local_lock();
        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();

        auto begin_ret = this->st_ctrl().update_global_begin(blk_glk, is_remotely_updated_ts);

        CMPTH_P_LOG_DEBUG(P,
            "Updating block state and data."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "is_remotely_updated", begin_ret.is_remotely_updated
        ,   "is_dirty", begin_ret.is_dirty
        ,   "needs_protect_before", begin_ret.needs_protect_before
        ,   "needs_protect_after", begin_ret.needs_protect_after
        ,   "is_write_protected", begin_ret.is_write_protected
        ,   "needs_local_copy", begin_ret.needs_local_copy
        ,   "needs_local_comp", begin_ret.needs_local_comp
        ,   "old_owner", blk_glk.prev_owner()
        ,   "old_last_writer_proc", blk_glk.last_writer_proc()
        );

        auto merge_ret = this->dt_ctrl().update_merge(blk_glk, begin_ret);

        this->st_ctrl().update_global_end(blk_glk, begin_ret, merge_ret);

        // Note: new_owner could be the same as old_owner in the past.
        const auto new_owner = merge_ret.is_migrated ? this_proc : blk_glk.prev_owner();

        const auto new_last_writer_proc = merge_ret.is_written ? this_proc : blk_glk.last_writer_proc();

        CMPTH_P_LOG_DEBUG(P,
            "Updated block state and data."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "is_remotely_updated", begin_ret.is_remotely_updated
        ,   "is_dirty", begin_ret.is_dirty
        ,   "needs_protect_before", begin_ret.needs_protect_before
        ,   "needs_protect_after", begin_ret.needs_protect_after
        ,   "is_write_protected", begin_ret.is_write_protected
        ,   "needs_local_copy", begin_ret.needs_local_copy
        ,   "needs_local_comp", begin_ret.needs_local_comp
        ,   "is_written", merge_ret.is_written
        ,   "is_migrated", merge_ret.is_migrated
        ,   "old_owner", blk_glk.prev_owner()
        ,   "new_owner", new_owner
        ,   "old_last_writer_proc", blk_glk.last_writer_proc()
        ,   "new_last_writer_proc", new_last_writer_proc
        );

        return { begin_ret.is_write_protected, merge_ret.is_written, new_owner, new_last_writer_proc };
    }

    typename state_ctrl_type::invalidate_result invalidate(blk_local_lock_type& blk_llk)
    {
        auto st_ret = this->st_ctrl().invalidate(blk_llk);
        if (st_ret.needs_protect) {
            CMPTH_P_PROF_SCOPE(P, mprotect_invalidate);
            this->dt_ctrl().protect_invalid(blk_llk);
        }
        return st_ret;
    }

    void start_pin(blk_local_lock_type& blk_llk)
    {
        this->st_ctrl().set_pinned(blk_llk);
    }
    void end_pin(blk_local_lock_type& blk_llk)
    {
        this->st_ctrl().set_unpinned(blk_llk);
    }

private:
    state_ctrl_type& st_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->st_ctrl_ptr_);
        return *this->st_ctrl_ptr_;
    }
    data_ctrl_type& dt_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->dt_ctrl_ptr_);
        return *this->dt_ctrl_ptr_;
    }

    state_ctrl_ptr_type st_ctrl_ptr_;
    data_ctrl_ptr_type  dt_ctrl_ptr_;
};

} // namespace medsm3
} // namespace menps

