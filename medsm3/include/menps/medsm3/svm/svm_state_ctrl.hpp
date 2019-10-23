
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_state_ctrl
    : public basic_state_ctrl<P>
{
    using base = basic_state_ctrl<P>;
    using blk_local_lock_type = typename P::blk_local_lock_type;
    
    using blk_state_type = typename P::blk_state_type;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;
    using wr_count_ctrl_ptr_type = typename P::wr_count_ctrl_ptr_type;

    using constants_type = typename P::constants_type;

public:
    explicit svm_state_ctrl(
        segment_set_ptr_type    seg_set_ptr
    ,   wr_count_ctrl_ptr_type  wr_count_ctrl_ptr
    )
        : base{fdn::move(wr_count_ctrl_ptr)}
        , seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool is_migration_enabled() const noexcept {
        return constants_type::is_migration_enabled;
    }

private:
    friend basic_state_ctrl<P>;

    void set_local_state(blk_local_lock_type& blk_llk, const blk_state_type state) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        blk_le.state = state;
    }
    blk_state_type get_local_state(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        return blk_le.state;
    }

    segment_set_type& seg_set() noexcept {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        return *this->seg_set_ptr_;
    }

    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps

