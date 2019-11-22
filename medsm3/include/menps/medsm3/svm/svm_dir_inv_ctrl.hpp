
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_dir_inv_ctrl
    : public P::inv_ctrl_base_type
    // = dir_inv_ctrl
{
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;

    template <typename T>
    using remote_ptr = typename rma_itf_type::template remote_ptr<T>;

public:
    explicit svm_dir_inv_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool read_local_inv_flag(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        return *seg.get_local_inv_flag_ptr(blk_llk);
    }
    void reset_local_inv_flag(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        *seg.get_local_inv_flag_ptr(blk_llk) = false;
    }
    remote_ptr<bool> get_remote_inv_flag_ptr(
        const proc_id_type      proc
    ,   blk_local_lock_type&    blk_llk
    ) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        return seg.get_remote_inv_flag_ptr(proc, blk_llk);
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

