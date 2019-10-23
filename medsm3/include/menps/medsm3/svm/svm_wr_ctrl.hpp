
#pragma once

#include <menps/medsm3/writer/basic_wr_ctrl.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_wr_ctrl
    : public basic_wr_ctrl<P>
{
private:
    using base = basic_wr_ctrl<P>;

    using flag_set_ref_type = typename P::flag_set_ref_type;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;

    using rd_ctrl_ptr_type = typename P::rd_ctrl_ptr_type;

    using constants_type = typename P::constants_type;

public:
    explicit svm_wr_ctrl(segment_set_ptr_type seg_set_ptr, rd_ctrl_ptr_type rd_ctrl_ptr)
        : base{fdn::move(rd_ctrl_ptr)}
        , seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool is_fast_release_enabled() const noexcept {
        return constants_type::is_fast_release_enabled;
    }
    bool is_signature_enabled() const noexcept {
        return constants_type::is_signature_enabled;
    }

    flag_set_ref_type get_flag_set_ref() const noexcept {
        return flag_set_ref_type{*seg_set_ptr_};
    }

private:
    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps

