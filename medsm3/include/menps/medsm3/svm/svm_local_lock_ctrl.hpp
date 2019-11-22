
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_local_lock_ctrl
{
    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_id_type = typename P::blk_id_type;

    using segment_set_ptr_type = typename P::segment_set_ptr_type;

    using ult_itf_type = typename P::ult_itf_type;

    using blk_mutex_type = typename P::blk_mutex_type;
    using blk_unique_lock_type = typename ult_itf_type::template unique_lock<blk_mutex_type>;

    using com_itf_type = typename P::com_itf_type;

public:
    explicit svm_local_lock_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    MEFDN_ALWAYS_INLINE
    blk_local_lock_type get_local_lock(const blk_id_type blk_id) {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        auto& seg_set = *this->seg_set_ptr_;
        auto& seg = seg_set.segment_of_blk_id(blk_id);
        auto& blk_le = seg.get_local_entry_of_blk_id(blk_id);
        return blk_local_lock_type{
            seg_set.get_com_itf()
        ,   blk_id
        ,   blk_unique_lock_type{ blk_le.mtx }
        ,   seg.get_blk_size()
        ,   seg.get_tag_from_subindex(blk_id.sidx)
        };
    }

    // TODO: This is required only for ts_rd_ctrl::fence_acquire_all().
    com_itf_type& get_com_itf() {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        return this->seg_set_ptr_->get_com_itf();
    }

private:
    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps

