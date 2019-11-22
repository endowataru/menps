
#pragma once

#include <menps/medsm3/reader/ts/ts_rd_ctrl.hpp>
#include <menps/medsm3/reader/ts/ts_home_ctrl.hpp>
#include <menps/medsm3/reader/ts/ts_inv_ctrl.hpp>
#include <menps/medsm3/reader/ts/ts_blk_global_lock.hpp>
#include <menps/medsm3/reader/ts/ts_sync_table.hpp>
#include <menps/medsm3/svm/svm_home_ctrl.hpp>
#include <menps/medsm3/svm/svm_ts_inv_ctrl.hpp>
#include <menps/medsm2/dsm/rd_set.hpp>
#include <menps/medsm2/dsm/rd_ts_state.hpp>
#include <menps/medsm2/dsm/sig_buffer.hpp>
#include <menps/medsm2/dsm/mtx_table.hpp>
#include <menps/medsm2/dsm/rel_sig.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv3_ts_md3_policy_base
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using constants_type = typename lv2_itf_type::constants_type;

    using com_itf_type = typename lv2_itf_type::com_itf_type;
    using ult_itf_type = typename lv2_itf_type::ult_itf_type;
    using assert_policy_type = typename ult_itf_type::assert_policy;
    using log_aspect_type = typename lv2_itf_type::log_aspect_type;

    using blk_id_type = typename lv2_itf_type::blk_id_type;

    using wr_ts_type = typename lv2_itf_type::wr_ts_type;
    using rd_ts_type = typename lv2_itf_type::rd_ts_type;
    using ts_interval_type = typename lv2_itf_type::ts_interval_type;
};

template <typename P>
struct ts_md3_rd_set_policy;

template <typename P>
struct ts_md3_rd_ts_state_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using rd_set_type = medsm2::rd_set<ts_md3_rd_set_policy<P>>;
};

template <typename P>
struct ts_md3_rd_set_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using typename lv3_ts_md3_policy_base<P>::ult_itf_type;
    using typename lv3_ts_md3_policy_base<P>::rd_ts_type;
    using atomic_wr_ts_type = typename ult_itf_type::template atomic<rd_ts_type>;
    using rd_ts_state_type = medsm2::rd_ts_state<ts_md3_rd_ts_state_policy<P>>;
    
    using size_type = fdn::size_t;

    static constexpr bool is_greater_rd_ts(const rd_ts_type a, const rd_ts_type b) noexcept {
        return a > b;
    }
};

template <typename P>
struct ts_md3_home_ctrl_policy;

template <typename P>
using ts_md3_home_ctrl = svm_home_ctrl<ts_md3_home_ctrl_policy<P>>;

template <typename P>
struct ts_md3_blk_global_lock_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using blk_global_lock_base_type = typename lv2_itf_type::blk_global_lock_base_type;
    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;
    using home_ctrl_type = ts_md3_home_ctrl<P>;
    using global_entry_type = typename lv2_itf_type::segment_type::global_entry_type;
};

template <typename P>
struct ts_md3_home_ctrl_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = ts_md3_home_ctrl<P>;
    using home_ctrl_base_type = ts_home_ctrl<ts_md3_home_ctrl_policy>;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;
    using blk_global_lock_type = ts_blk_global_lock<ts_md3_blk_global_lock_policy<P>>;

    using segment_set_type = typename lv2_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv2_itf_type::segment_set_ptr_type;

    using global_entry_type = typename lv2_itf_type::segment_type::global_entry_type;
};

template <typename P>
struct ts_md3_sig_buffer_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using size_type = fdn::size_t;

    using typename lv3_ts_md3_policy_base<P>::com_itf_type;
    using typename lv3_ts_md3_policy_base<P>::wr_ts_type;
    using typename lv3_ts_md3_policy_base<P>::rd_ts_type;
    using typename lv3_ts_md3_policy_base<P>::ts_interval_type;
    using typename lv3_ts_md3_policy_base<P>::blk_id_type;

    struct wn_entry_type {
        typename com_itf_type::proc_id_type home_proc;
        blk_id_type                         blk_id;
        ts_interval_type                    ts_intvl;
    };

    using wn_vector_type = std::vector<wn_entry_type>;   

    static constexpr wr_ts_type make_init_wr_ts() noexcept {
        return 0;
    }
    static wr_ts_type get_wr_ts(const wn_entry_type& wn) noexcept {
        return wn.ts_intvl.wr_ts;
    }
    static constexpr bool is_greater_wr_ts(const wr_ts_type a, const wr_ts_type b) noexcept {
        return a > b;
    }
};

template <typename P>
struct ts_md3_inv_ctrl_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = svm_ts_inv_ctrl<ts_md3_inv_ctrl_policy>;
    using inv_ctrl_base_type = ts_inv_ctrl<ts_md3_inv_ctrl_policy>;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;
    using ts_type = typename lv2_itf_type::ts_type;

    using rd_ts_state_type = typename ts_md3_rd_set_policy<P>::rd_ts_state_type;

    using wn_entry_type = typename ts_md3_sig_buffer_policy<P>::wn_entry_type;

    using blk_global_lock_type = typename ts_md3_home_ctrl_policy<P>::blk_global_lock_type;

    using segment_set_type = typename lv2_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv2_itf_type::segment_set_ptr_type;

    static constexpr bool is_greater_ts(const ts_type a, const ts_type b) noexcept {
        return a > b;
    }
};

template <typename P>
struct ts_md3_rd_ctrl_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = ts_rd_ctrl<ts_md3_rd_ctrl_policy>;
    using rd_set_type = typename ts_md3_rd_ts_state_policy<P>::rd_set_type;
    using rd_ts_state_type = typename ts_md3_rd_set_policy<P>::rd_ts_state_type;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;

    using local_lock_ctrl_type = typename lv2_itf_type::local_lock_ctrl_type;
    using local_lock_ctrl_ptr_type = typename lv2_itf_type::local_lock_ctrl_ptr_type;
    using state_data_ctrl_type = typename lv2_itf_type::state_data_ctrl_type;
    using state_data_ctrl_ptr_type = typename lv2_itf_type::state_data_ctrl_ptr_type;
    using inv_ctrl_type = typename ts_md3_inv_ctrl_policy<P>::derived_type;
    using inv_ctrl_ptr_type = fdn::unique_ptr<inv_ctrl_type>;
    using home_ctrl_type = typename ts_md3_home_ctrl_policy<P>::derived_type;
    using home_ctrl_ptr_type = fdn::unique_ptr<home_ctrl_type>;

    using blk_global_lock_type = typename ts_md3_home_ctrl_policy<P>::blk_global_lock_type;

    using sig_buffer_type = medsm2::sig_buffer<ts_md3_sig_buffer_policy<P>>;
    using sig_buf_set_type = std::vector<sig_buffer_type>;

    using wn_entry_type = typename ts_md3_sig_buffer_policy<P>::wn_entry_type;
};

template <typename P>
struct ts_md3_rel_sig_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using size_type = fdn::size_t;

    using sig_buffer_type = typename ts_md3_rd_ctrl_policy<P>::sig_buffer_type;

    using wr_ts_type = typename lv2_itf_type::wr_ts_type;
    using wn_vector_type = typename ts_md3_sig_buffer_policy<P>::wn_vector_type;
};

template <typename P>
struct ts_md3_mtx_table_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;

public:
    using sig_buffer_type = typename ts_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using lock_table_type = typename lv2_itf_type::global_lock_table_type;

    using mtx_id_type = typename lv2_itf_type::mtx_id_type;
    using size_type = fdn::size_t;

    static constexpr bool is_valid_mutex_id(const mtx_id_type mtx_id) noexcept {
        return mtx_id != 0;
    }
    using p2p_tag_type = int; // TODO
    static p2p_tag_type get_tag_from_lock_id(const mtx_id_type lk_id) {
        return static_cast<p2p_tag_type>(lk_id);
    }
};

template <typename P>
struct ts_md3_sync_table_policy
    : lv3_ts_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;

public:
    using sig_buffer_type = typename ts_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using sig_buf_set_type = typename ts_md3_rd_ctrl_policy<P>::sig_buf_set_type;
    using mtx_id_type = typename ts_md3_mtx_table_policy<P>::mtx_id_type;
    using mtx_table_type = medsm2::mtx_table<ts_md3_mtx_table_policy<P>>;
    using prof_aspect_type = typename lv2_itf_type::prof_aspect_type;
    using rel_sig_type = medsm2::rel_sig<ts_md3_rel_sig_policy<P>>;
    using id_allocator_type = typename lv2_itf_type::id_allocator_type;
};

template <typename P>
struct lv3_ts_md3_itf
    : P::lv2_itf_type
{
    using rd_ctrl_type = typename ts_md3_rd_ctrl_policy<P>::derived_type;
    using rd_ctrl_ptr_type = fdn::unique_ptr<rd_ctrl_type>;
    using rd_ts_state_type = typename ts_md3_rd_set_policy<P>::rd_ts_state_type;
    using sig_buffer_type = typename ts_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using sig_buf_set_type = typename ts_md3_rd_ctrl_policy<P>::sig_buf_set_type;
    using wn_entry_type = typename ts_md3_sig_buffer_policy<P>::wn_entry_type;
    using wn_vector_type = typename ts_md3_sig_buffer_policy<P>::wn_vector_type;
    using blk_global_lock_type = typename ts_md3_home_ctrl_policy<P>::blk_global_lock_type;
    using inv_ctrl_type = typename ts_md3_rd_ctrl_policy<P>::inv_ctrl_type;
    using inv_ctrl_ptr_type = typename ts_md3_rd_ctrl_policy<P>::inv_ctrl_ptr_type;
    using home_ctrl_type = typename ts_md3_rd_ctrl_policy<P>::home_ctrl_type;
    using home_ctrl_ptr_type = typename ts_md3_rd_ctrl_policy<P>::home_ctrl_ptr_type;

    using rel_sig_type = typename ts_md3_sync_table_policy<P>::rel_sig_type;
    using sync_table_type = ts_sync_table<ts_md3_sync_table_policy<P>>;
    using sync_table_ptr_type = fdn::unique_ptr<sync_table_type>;
};

} // namespace medsm3
} // namespace menps

