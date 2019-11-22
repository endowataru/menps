
#pragma once

#include <menps/medsm3/data/basic_wr_count_ctrl.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_wr_count_ctrl
    : public basic_wr_count_ctrl<P>
{
    using wr_count_type = typename P::wr_count_type;
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;
    using local_entry_type = typename segment_set_type::local_entry_type;

    using constants_type = typename P::constants_type;

public:
    explicit svm_wr_count_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool is_lazy_merge_enabled() const noexcept {
        return constants_type::is_lazy_merge_enabled;
    }
    bool is_needs_local_comp_enabled() const noexcept {
        return constants_type::is_needs_local_comp_enabled;
    }
    bool is_fast_release_enabled() const noexcept {
        return constants_type::is_fast_release_enabled;
    }

    wr_count_type get_wr_count_threshold(blk_local_lock_type& /*blk_llk*/) const noexcept {
        return constants_type::wr_count_threshold;
    }
    wr_count_type get_max_fast_rel_threshold(blk_local_lock_type& /*blk_llk*/) const noexcept {
        return constants_type::max_fast_rel_threshold;
    }

    wr_count_type get_wr_count(blk_local_lock_type& blk_llk) noexcept {
        return this->seg_set().get_local_entry_of(blk_llk, &local_entry_type::wr_count);
    }
    void set_wr_count(blk_local_lock_type& blk_llk, const wr_count_type wr_count) noexcept {
        this->seg_set().set_local_entry_of(blk_llk, &local_entry_type::wr_count, wr_count);
    }

    bool get_written_last(blk_local_lock_type& blk_llk) noexcept {
        return this->seg_set().get_local_entry_of(blk_llk, &local_entry_type::is_written_last);
    }
    void set_written_last(blk_local_lock_type& blk_llk, const bool is_written_last) noexcept {
        this->seg_set().set_local_entry_of(blk_llk, &local_entry_type::is_written_last, is_written_last);
    }

    wr_count_type get_fast_rel_count(blk_local_lock_type& blk_llk) noexcept {
        return this->seg_set().get_local_entry_of(blk_llk, &local_entry_type::fast_rel_count);
    }
    void set_fast_rel_count(blk_local_lock_type& blk_llk, const wr_count_type fast_rel_count) noexcept {
        this->seg_set().set_local_entry_of(blk_llk, &local_entry_type::fast_rel_count, fast_rel_count);
    }

    wr_count_type get_fast_rel_threshold(blk_local_lock_type& blk_llk) noexcept {
        return this->seg_set().get_local_entry_of(blk_llk, &local_entry_type::fast_rel_threshold);
    }
    void set_fast_rel_threshold(blk_local_lock_type& blk_llk, const wr_count_type fast_rel_threshold) noexcept {
        this->seg_set().set_local_entry_of(blk_llk, &local_entry_type::fast_rel_threshold, fast_rel_threshold);
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

