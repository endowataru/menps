
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_pin_ctrl
{
    using rd_ctrl_type = typename P::rd_ctrl_type;
    using wr_ctrl_type = typename P::wr_ctrl_type;
    using wr_ctrl_ptr_type = typename P::wr_ctrl_ptr_type;

    using blk_id_type = typename P::blk_id_type;
    using blk_local_lock_type = typename P::blk_local_lock_type;

public:
    explicit basic_pin_ctrl(wr_ctrl_ptr_type wr_ctrl_ptr)
        : wr_ctrl_ptr_{fdn::move(wr_ctrl_ptr)}
    { }

    void start_pin(blk_local_lock_type& blk_llk)
    {
        const auto read_upgraded MEFDN_MAYBE_UNUSED =
            this->rd_ctrl().try_start_read(blk_llk);
        const auto write_upgraded MEFDN_MAYBE_UNUSED =
            this->wr_ctrl().try_start_write(blk_llk);
        
        auto& sd_ctrl = this->rd_ctrl().state_data_ctrl();
        sd_ctrl.start_pin(blk_llk);
    }

    void end_pin(blk_local_lock_type& blk_llk)
    {
        auto& sd_ctrl = this->rd_ctrl().state_data_ctrl();
        sd_ctrl.end_pin(blk_llk);
    }

public:
    rd_ctrl_type& rd_ctrl() noexcept {
        return this->wr_ctrl().rd_ctrl();
    }
    wr_ctrl_type& wr_ctrl() noexcept {
        CMPTH_P_ASSERT(P, this->wr_ctrl_ptr_);
        return *this->wr_ctrl_ptr_;
    }

private:
    wr_ctrl_ptr_type wr_ctrl_ptr_;
};

} // namespace medsm3
} // namespace menps

