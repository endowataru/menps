
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_flag_set_ref
{
    using segment_set_type = typename P::segment_set_type;
    using blk_id_type = typename P::blk_id_type;

    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::template unique_lock<mutex_type>;

public:
    explicit svm_flag_set_ref(segment_set_type& seg_set)
        : seg_set_(seg_set)
    { }

    bool try_set_flag(const blk_id_type blk_id) {
        auto& seg = this->seg_set_.segment_of_blk_id(blk_id);
        auto& blk_le = seg.get_local_entry_of_blk_id(blk_id);
        if (blk_le.flag) {
            return false;
        }
        else {
            blk_le.flag = true;
            return true;
        }
    }
    void unset_flag(const blk_id_type blk_id) {
        auto& seg = this->seg_set_.segment_of_blk_id(blk_id);
        auto& blk_le = seg.get_local_entry_of_blk_id(blk_id);
        blk_le.flag = false;
    }

    unique_lock_type get_flags_lock() {
        return this->seg_set_.get_flag_set_lock();
    }

private:
    segment_set_type& seg_set_;
};

} // namespace medsm3
} // namespace menps

