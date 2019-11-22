
#pragma once

#include <menps/medsm3/svm/svm_wr_ctrl.hpp>
#include <menps/medsm3/svm/svm_flag_set_ref.hpp>
#include <menps/medsm3/writer/basic_pin_ctrl.hpp>
#include <menps/medsm2/dsm/wr_set.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv4_md3_policy_base
{
private:
    using lv3_itf_type = typename P::lv3_itf_type;
    
public:
    using constants_type = typename lv3_itf_type::constants_type;

    using com_itf_type = typename lv3_itf_type::com_itf_type;
    using ult_itf_type = typename lv3_itf_type::ult_itf_type;
    using assert_policy_type = typename ult_itf_type::assert_policy;
    using log_aspect_type = typename lv3_itf_type::log_aspect_type;

    using prof_aspect_type = typename lv3_itf_type::prof_aspect_type;

    using blk_id_type = typename lv3_itf_type::blk_id_type;
    using blk_local_lock_type = typename lv3_itf_type::blk_local_lock_type;
};

template <typename P>
struct md3_wr_set_policy
    : lv4_md3_policy_base<P>
{
private:
    using lv3_itf_type = typename P::lv3_itf_type;
    
public:
    using size_type = fdn::size_t;

    using wn_entry_type = typename lv3_itf_type::wn_entry_type;
    using wn_vector_type = typename lv3_itf_type::wn_vector_type;

    using wr_set_gen_type = mefdn::ptrdiff_t;
    
    using rd_ts_state_type = typename lv3_itf_type::rd_ts_state_type;
    using ts_type = typename lv3_itf_type::ts_type;
};

template <typename P>
struct md3_flag_set_ref_policy
    : lv4_md3_policy_base<P>
{
private:
    using lv3_itf_type = typename P::lv3_itf_type;
    
public:
    using segment_set_type = typename lv3_itf_type::segment_set_type;
};

template <typename P>
struct md3_wr_ctrl_policy
    : lv4_md3_policy_base<P>
{
private:
    using lv3_itf_type = typename P::lv3_itf_type;
    
public:
    using derived_type = svm_wr_ctrl<md3_wr_ctrl_policy>;
    
    using rd_ctrl_type = typename lv3_itf_type::rd_ctrl_type;
    using rd_ctrl_ptr_type = typename lv3_itf_type::rd_ctrl_ptr_type;
    using rd_ts_state_type = typename lv3_itf_type::rd_ts_state_type;

    using wr_set_type = medsm2::wr_set<md3_wr_set_policy<P>>;
    using sig_buffer_type = typename lv3_itf_type::sig_buffer_type;
    using rel_sig_type = typename lv3_itf_type::rel_sig_type;

    using wn_entry_type = typename lv3_itf_type::wn_entry_type;
    using wn_vector_type = typename lv3_itf_type::wn_vector_type;

    using flag_set_ref_type = svm_flag_set_ref<md3_flag_set_ref_policy<P>>;
    using segment_set_type = typename lv3_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv3_itf_type::segment_set_ptr_type;
};

template <typename P>
struct md3_pin_ctrl_policy
    : lv4_md3_policy_base<P>
{
private:
    using lv3_itf_type = typename P::lv3_itf_type;
    
public:
    using derived_type = basic_pin_ctrl<md3_pin_ctrl_policy>;

    using rd_ctrl_type = typename lv3_itf_type::rd_ctrl_type;
    using wr_ctrl_type = typename md3_wr_ctrl_policy<P>::derived_type;
    using wr_ctrl_ptr_type = fdn::unique_ptr<wr_ctrl_type>;
};

template <typename P>
struct lv4_md3_itf
    : P::lv3_itf_type
{
    using wr_ctrl_type = typename md3_wr_ctrl_policy<P>::derived_type;
    using pin_ctrl_type = typename md3_pin_ctrl_policy<P>::derived_type;
};

} // namespace medsm3
} // namespace menps

