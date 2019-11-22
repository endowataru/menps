
#pragma once

#include <menps/medsm3/data/basic_state_ctrl.hpp>
#include <menps/medsm3/data/basic_state_data_ctrl.hpp>
#include <menps/medsm3/data/basic_merge_policy.hpp>
#include <menps/medsm3/svm/svm_data_ctrl.hpp>
#include <menps/medsm3/svm/svm_state_ctrl.hpp>
#include <menps/medsm3/svm/svm_wr_count_ctrl.hpp>
#include <menps/medsm3/svm/svm_local_lock_ctrl.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv2_md3_policy_base
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using blk_local_lock_type = typename lv1_itf_type::blk_local_lock_type;
    using blk_global_lock_base_type = typename lv1_itf_type::blk_global_lock_base_type;
    using com_itf_type = typename lv1_itf_type::com_itf_type;

    using ult_itf_type = typename lv1_itf_type::ult_itf_type;
    using assert_policy_type = typename ult_itf_type::assert_policy;
    using log_aspect_type = typename lv1_itf_type::log_aspect_type;

    using prof_aspect_type = typename lv1_itf_type::prof_aspect_type;

    using constants_type = typename lv1_itf_type::constants_type;
};

template <typename P>
struct md3_wr_count_ctrl_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using derived_type = svm_wr_count_ctrl<md3_wr_count_ctrl_policy>;
    using wr_count_type = typename lv1_itf_type::wr_count_type;
    using segment_set_type = typename lv1_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv1_itf_type::segment_set_ptr_type;
};

template <typename P>
struct md3_state_ctrl_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using derived_type = svm_state_ctrl<md3_state_ctrl_policy>;
    using wr_count_ctrl_type = typename md3_wr_count_ctrl_policy<P>::derived_type;
    using wr_count_ctrl_ptr_type = fdn::unique_ptr<wr_count_ctrl_type>;
    using blk_state_type = typename lv1_itf_type::blk_state_type;
    using segment_set_type = typename lv1_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv1_itf_type::segment_set_ptr_type;
};

template <typename P>
struct md3_merge_policy_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using blk_local_lock_type = typename lv1_itf_type::blk_local_lock_type;
    using constants_type = typename lv1_itf_type::constants_type;
};

template <typename P>
struct md3_data_ctrl_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using derived_type = svm_data_ctrl<md3_data_ctrl_policy>;
    using merge_policy_type = basic_merge_policy<md3_merge_policy_policy<P>>;
    using segment_set_type = typename lv1_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv1_itf_type::segment_set_ptr_type;
};


template <typename P>
struct md3_state_data_ctrl_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;
    
public:
    using derived_type = basic_state_data_ctrl<md3_state_data_ctrl_policy>;
    using data_ctrl_type = typename md3_data_ctrl_policy<P>::derived_type;
    using data_ctrl_ptr_type = fdn::unique_ptr<data_ctrl_type>;
    using state_ctrl_type = typename md3_state_ctrl_policy<P>::derived_type;
    using state_ctrl_ptr_type = fdn::unique_ptr<state_ctrl_type>;
};

template <typename P>
struct md3_local_lock_ctrl_policy
    : lv2_md3_policy_base<P>
{
private:
    using lv1_itf_type = typename P::lv1_itf_type;

public:
    using blk_local_lock_type = typename lv1_itf_type::blk_local_lock_type;
    using segment_set_ptr_type = typename lv1_itf_type::segment_set_ptr_type;
    using ult_itf_type = typename lv1_itf_type::ult_itf_type;
    using blk_mutex_type = typename lv1_itf_type::blk_mutex_type;
    using blk_id_type = typename lv1_itf_type::blk_id_type;
};

template <typename P>
struct lv2_md3_itf
    : P::lv1_itf_type
{
    using wr_count_ctrl_type = typename md3_wr_count_ctrl_policy<P>::derived_type;
    using wr_count_ctrl_ptr_type = typename md3_state_ctrl_policy<P>::wr_count_ctrl_ptr_type;
    using data_ctrl_type = typename md3_data_ctrl_policy<P>::derived_type;
    using data_ctrl_ptr_type = typename md3_state_data_ctrl_policy<P>::data_ctrl_ptr_type;
    using state_ctrl_type = typename md3_state_ctrl_policy<P>::derived_type;
    using state_ctrl_ptr_type = typename md3_state_data_ctrl_policy<P>::state_ctrl_ptr_type;
    using state_data_ctrl_type = typename md3_state_data_ctrl_policy<P>::derived_type;
    using state_data_ctrl_ptr_type = fdn::unique_ptr<state_data_ctrl_type>;
    using local_lock_ctrl_type = svm_local_lock_ctrl<md3_local_lock_ctrl_policy<P>>;
    using local_lock_ctrl_ptr_type = fdn::unique_ptr<local_lock_ctrl_type>;
};

} // namespace medsm3
} // namespace menps

