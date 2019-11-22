
#pragma once

#include <menps/medsm3/svm/basic_svm_space.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv5_md3_policy_base
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;
    
public:
    using com_itf_type = typename lv4_itf_type::com_itf_type;
    using ult_itf_type = typename lv4_itf_type::ult_itf_type;
    using assert_policy_type = typename ult_itf_type::assert_policy;
    using log_aspect_type = typename lv4_itf_type::log_aspect_type;

    using prof_aspect_type = typename lv4_itf_type::prof_aspect_type;

    using blk_id_type = typename lv4_itf_type::blk_id_type;
};

template <typename P>
struct md3_space_policy
    : lv5_md3_policy_base<P>
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;

public:
    using derived_type = basic_svm_space<md3_space_policy>;

    using segment_set_type = typename lv4_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv4_itf_type::segment_set_ptr_type;
    using space_base_type = typename P::space_base_type;

    using pin_ctrl_type = typename lv4_itf_type::pin_ctrl_type;
    using pin_ctrl_ptr_type = fdn::unique_ptr<pin_ctrl_type>;
    using wr_ctrl_type = typename lv4_itf_type::wr_ctrl_type;
    using rd_ctrl_type = typename lv4_itf_type::rd_ctrl_type;
    using local_lock_ctrl_type = typename lv4_itf_type::local_lock_ctrl_type;

    using sync_table_type = typename lv4_itf_type::sync_table_type;
    using sync_table_ptr_type = typename lv4_itf_type::sync_table_ptr_type;

    using mtx_id_type = typename lv4_itf_type::mtx_id_type;
};

template <typename P>
struct lv5_md3_itf
    : P::lv4_itf_type
{
    using space = typename md3_space_policy<P>::derived_type;
    using pin_ctrl_type = typename md3_space_policy<P>::pin_ctrl_type;
    using sync_table_type = typename md3_space_policy<P>::sync_table_type;
};

} // namespace medsm3
} // namespace menps

