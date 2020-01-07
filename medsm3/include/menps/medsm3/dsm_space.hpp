
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dsm_space
    : public P::space_base_type
    // = medsm2::svm_space_base
{
    CMPTH_DEFINE_DERIVED(P)

    using base = typename P::space_base_type;

    using blk_id_type = typename P::blk_id_type;
    using mtx_id_type = typename P::mtx_id_type;

    using pin_ctrl_type = typename P::pin_ctrl_type;
    using pin_ctrl_ptr_type = typename P::pin_ctrl_ptr_type;
    
    using wr_ctrl_type = typename P::wr_ctrl_type;
    using rd_ctrl_type = typename P::rd_ctrl_type;
    using local_lock_ctrl_type = typename P::local_lock_ctrl_type;

    using sync_table_type = typename P::sync_table_type;
    using sync_table_ptr_type = typename P::sync_table_ptr_type;

    using sig_buffer_type = typename P::sig_buffer_type;
    using sig_buf_set_type = typename P::sig_buf_set_type;

public:
    explicit dsm_space(pin_ctrl_ptr_type pin_ctrl, sync_table_ptr_type sync_tbl)
        : pin_ctrl_{fdn::move(pin_ctrl)}
        , sync_tbl_{fdn::move(sync_tbl)}
    { }

protected:
    #if 0
    MEFDN_NODISCARD
    bool try_start_read(const blk_id_type blk_id)
    {
        return this->rd_ctrl().try_start_read(blk_id);
    }

    MEFDN_NODISCARD
    bool try_start_write(const blk_id_type blk_id)
    {
        return this->wr_ctrl().try_start_write(blk_id);
    }
    #endif

    MEFDN_NODISCARD
    bool try_upgrade_block(const blk_id_type blk_id)
    {
        auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
        if (this->rd_ctrl().try_start_read(blk_llk)) {
            // This block was exactly read at this time.
            return true;
        }
        if (this->wr_ctrl().try_start_write(blk_llk)) {
            // This block was or is now writable.
            // This behavior allows the situation
            // where the block has already been writable.
            // because other threads may have changed the state to writable.
            return true;
        }
        return false;
    }
    MEFDN_NODISCARD
    bool try_upgrade_block_with_flag(const blk_id_type blk_id, const bool is_write)
    {
        auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
        const auto is_read_done = this->rd_ctrl().try_start_read(blk_llk);
        if (!is_write) {
            return is_read_done;
        }
        return this->wr_ctrl().try_start_write(blk_llk);
    }

    void start_pin_block(const blk_id_type blk_id)
    {
        auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
        this->pin_ctrl_->start_pin(blk_llk);

        CMPTH_P_LOG_DEBUG(P, "Pinned block.", "blk_id", blk_id.to_str());
    }
    void end_pin_block(const blk_id_type blk_id)
    {
        auto blk_llk = this->local_lock_ctrl().get_local_lock(blk_id);
        this->pin_ctrl_->end_pin(blk_llk);

        CMPTH_P_LOG_DEBUG(P, "Unpinned block.", "blk_id", blk_id.to_str());
    }

    #if 0
    template <typename Op>
    atomic_ret atomic_op(blk_id_type blk_id, Op op, order order)
    {
        if (order & release) {
            auto sig = wr_ctrl_.fence_release();
            sync_tbl_.atomic_release(blk_id, sig);
        }
        auto ret = atom_ctrl_.atomic_op(blk_id, op);
        if (order & acquire) {
            auto sig = sync_tbl_.atomic_acquire(blk_id);
            rd_ctrl_.fence_acquire(sig);
        }
        return ret;
    }
    #endif

public:
    virtual void barrier() override
    {
        CMPTH_P_PROF_SCOPE(P, barrier);
        CMPTH_P_LOG_INFO(P, "Entering DSM barrier.");

        sig_buffer_type sig;
        {
            CMPTH_P_PROF_SCOPE(P, barrier_release);
            sig = this->wr_ctrl().fence_release(true);
        }
        sig_buf_set_type sig_set;
        {
            CMPTH_P_PROF_SCOPE(P, barrier_allgather);
            sig_set = this->sync_tbl().exchange_sig(sig);
        }
        {
            CMPTH_P_PROF_SCOPE(P, barrier_acquire);
            this->rd_ctrl().fence_acquire_all(sig_set);
        }

        CMPTH_P_LOG_INFO(P, "Exiting DSM barrier.");
    }

    virtual mtx_id_type allocate_mutex() override {
        return this->sync_tbl().allocate_mutex();
    }
    virtual void deallocate_mutex(const mtx_id_type mtx_id) override {
        this->sync_tbl().deallocate_mutex(mtx_id);
    }

    virtual void lock_mutex(mtx_id_type mtx_id) override
    {
        auto sig = this->sync_tbl().lock_mutex(mtx_id);
        this->rd_ctrl().fence_acquire(sig);
    }

    virtual void unlock_mutex(mtx_id_type mtx_id) override
    {
        auto sig = this->wr_ctrl().fence_release(false);
        this->sync_tbl().unlock_mutex(mtx_id, sig);
    }
 
    virtual void start_release_thread() override
    {
        // TODO
    }
    virtual void stop_release_thread() override
    {
        // TODO
    }

private:
    rd_ctrl_type& rd_ctrl() noexcept { return this->wr_ctrl().rd_ctrl(); }
    wr_ctrl_type& wr_ctrl() noexcept { return this->pin_ctrl_->wr_ctrl(); }
    local_lock_ctrl_type& local_lock_ctrl() noexcept {
        return this->rd_ctrl().local_lock_ctrl();
    }
    sync_table_type& sync_tbl() noexcept {
        CMPTH_P_ASSERT(P, this->sync_tbl_);
        return *this->sync_tbl_;
    }

    pin_ctrl_ptr_type   pin_ctrl_;
    //atom_ctrl_type atom_ctrl_;
    sync_table_ptr_type sync_tbl_;
};

} // namespace medsm3
} // namespace menps

