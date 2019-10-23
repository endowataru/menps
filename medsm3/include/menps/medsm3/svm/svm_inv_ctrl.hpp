
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_inv_ctrl
    : public P::inv_ctrl_base_type
    // ts_inv_ctrl or dir_inv_ctrl
{
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using ts_interval_type = typename P::ts_interval_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using wn_entry_type = typename P::wn_entry_type;
    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;

public:
    explicit svm_inv_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    wn_entry_type get_local_wn(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        return { blk_le.wn_proc, blk_llk.blk_id(), blk_le.wn_intvl };
    }
    void set_local_wn(blk_local_lock_type& blk_llk, const proc_id_type owner, const ts_interval_type& ts_intvl) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        blk_le.wn_proc = owner;
        blk_le.wn_intvl = ts_intvl;
    }

private:
    segment_set_type& seg_set() noexcept {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        return *this->seg_set_ptr_;
    }

    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps

