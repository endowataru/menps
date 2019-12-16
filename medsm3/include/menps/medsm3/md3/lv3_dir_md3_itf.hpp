
#pragma once

#include <menps/medsm3/reader/dir/dir_rd_ctrl.hpp>
#include <menps/medsm3/reader/dir/dir_home_ctrl.hpp>
#include <menps/medsm3/reader/dir/dir_inv_ctrl.hpp>
#include <menps/medsm3/reader/dir/dir_blk_global_lock.hpp>
#include <menps/medsm3/reader/dir/dir_sync_table.hpp>
#include <menps/medsm3/reader/dir/dir_rel_sig.hpp>
#include <menps/medsm3/reader/dir/dir_sig_buffer.hpp>
#include <menps/medsm3/svm/svm_home_ctrl.hpp>
#include <menps/medsm3/svm/svm_dir_inv_ctrl.hpp>
#include <menps/medsm2/dsm/unordered_rd_set.hpp>
#include <menps/medsm2/dsm/rd_ts_state.hpp>
#include <menps/medsm2/dsm/mtx_table.hpp>
#include <unordered_set>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv3_dir_md3_policy_base
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using constants_type = typename lv2_itf_type::constants_type;

    using ult_itf_type = typename lv2_itf_type::ult_itf_type;
    using assert_aspect_type = typename ult_itf_type::assert_aspect;
    using log_aspect_type = typename lv2_itf_type::log_aspect_type;

    using blk_id_type = typename lv2_itf_type::blk_id_type;

    using sharer_map_type = typename lv2_itf_type::sharer_map_type;
};

template <typename P>
struct md3_blk_id_hash
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    using blk_id_type = typename lv2_itf_type::blk_id_type;
    using constants_type = typename lv2_itf_type::constants_type;

public:
    fdn::size_t operator() (const blk_id_type& blk_id) const noexcept {
        const fdn::size_t seg_id = blk_id.seg_id;
        const auto blk_abs_idx = (seg_id << constants_type::ln2_max_seg_size) + blk_id.sidx;
        return std::hash<fdn::size_t>()(blk_abs_idx);
    }
};

template <typename P>
struct dir_md3_rd_set_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using size_type = fdn::size_t;

    using typename lv3_dir_md3_policy_base<P>::blk_id_type;
    using blk_id_set_type = std::unordered_set<blk_id_type, md3_blk_id_hash<P>>;
};

template <typename P>
using dir_md3_rd_set = medsm2::unordered_rd_set<dir_md3_rd_set_policy<P>>;

template <typename P>
struct dir_md3_home_ctrl_policy;

template <typename P>
using dir_md3_home_ctrl = svm_home_ctrl<dir_md3_home_ctrl_policy<P>>;

template <typename P>
struct dir_md3_blk_global_lock_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using blk_global_lock_base_type = typename lv2_itf_type::blk_global_lock_base_type;
    using com_itf_type = typename lv2_itf_type::com_itf_type;
    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;
    using home_ctrl_type = dir_md3_home_ctrl<P>;
    using global_entry_type = typename lv2_itf_type::segment_type::global_entry_type;
};

template <typename P>
struct dir_md3_home_ctrl_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = dir_md3_home_ctrl<P>;
    using home_ctrl_base_type = dir_home_ctrl<dir_md3_home_ctrl_policy>;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;
    using blk_global_lock_type = dir_blk_global_lock<dir_md3_blk_global_lock_policy<P>>;

    using com_itf_type = typename lv2_itf_type::com_itf_type;

    using segment_set_type = typename lv2_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv2_itf_type::segment_set_ptr_type;

    using global_entry_type = typename lv2_itf_type::segment_type::global_entry_type;
};

template <typename P>
struct dir_md3_inv_ctrl_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = svm_dir_inv_ctrl<dir_md3_inv_ctrl_policy>;
    using inv_ctrl_base_type = dir_inv_ctrl<dir_md3_inv_ctrl_policy>;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;

    using rd_ts_state_type = typename dir_md3_rd_set<P>::rd_ts_state;

    using com_itf_type = typename lv2_itf_type::com_itf_type;

    using blk_global_lock_type = typename dir_md3_home_ctrl_policy<P>::blk_global_lock_type;

    using segment_set_type = typename lv2_itf_type::segment_set_type;
    using segment_set_ptr_type = typename lv2_itf_type::segment_set_ptr_type;
};

template <typename P>
struct dir_md3_rd_ctrl_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = dir_rd_ctrl<dir_md3_rd_ctrl_policy>;
    using rd_set_type = dir_md3_rd_set<P>;
    using rd_ts_state_type = typename rd_set_type::rd_ts_state;

    using blk_local_lock_type = typename lv2_itf_type::blk_local_lock_type;

    using local_lock_ctrl_type = typename lv2_itf_type::local_lock_ctrl_type;
    using local_lock_ctrl_ptr_type = typename lv2_itf_type::local_lock_ctrl_ptr_type;
    using state_data_ctrl_type = typename lv2_itf_type::state_data_ctrl_type;
    using state_data_ctrl_ptr_type = typename lv2_itf_type::state_data_ctrl_ptr_type;
    using inv_ctrl_type = typename dir_md3_inv_ctrl_policy<P>::derived_type;
    using inv_ctrl_ptr_type = fdn::unique_ptr<inv_ctrl_type>;
    using home_ctrl_type = typename dir_md3_home_ctrl_policy<P>::derived_type;
    using home_ctrl_ptr_type = fdn::unique_ptr<home_ctrl_type>;

    using blk_global_lock_type = typename dir_md3_home_ctrl_policy<P>::blk_global_lock_type;

    using sig_buffer_type = dir_sig_buffer;
    struct sig_buf_set_type { /* empty */ };
    struct wn_entry_type { /* empty */ };
    using wn_vector_type = std::vector<wn_entry_type>;
};

template <typename P>
struct dir_md3_rel_sig_policy
{
    using sig_buffer_type = typename dir_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using wn_vector_type = typename dir_md3_rd_ctrl_policy<P>::wn_vector_type;
};

template <typename P>
struct dir_md3_mtx_table_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;

public:
    using sig_buffer_type = typename dir_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using lock_table_type = typename lv2_itf_type::global_lock_table_type;

    using com_itf_type = typename lv2_itf_type::com_itf_type;
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
struct dir_md3_sync_table_policy
    : lv3_dir_md3_policy_base<P>
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;

public:
    using sig_buffer_type = typename dir_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using sig_buf_set_type = typename dir_md3_rd_ctrl_policy<P>::sig_buf_set_type;
    using mtx_id_type = typename dir_md3_mtx_table_policy<P>::mtx_id_type;
    using mtx_table_type = medsm2::mtx_table<dir_md3_mtx_table_policy<P>>;
    using prof_aspect_type = typename lv2_itf_type::prof_aspect_type;
    using rel_sig_type = dir_rel_sig<dir_md3_rel_sig_policy<P>>;
    using com_itf_type = typename lv2_itf_type::com_itf_type;
    using id_allocator_type = typename lv2_itf_type::id_allocator_type;
};

template <typename P>
struct lv3_dir_md3_itf
    : P::lv2_itf_type
{
    using rd_ctrl_type = typename dir_md3_rd_ctrl_policy<P>::derived_type;
    using rd_ctrl_ptr_type = fdn::unique_ptr<rd_ctrl_type>;
    using rd_ts_state_type = typename dir_md3_rd_ctrl_policy<P>::rd_ts_state_type;
    using wn_entry_type = typename dir_md3_rd_ctrl_policy<P>::wn_entry_type;
    using sig_buffer_type = typename dir_md3_rd_ctrl_policy<P>::sig_buffer_type;
    using sig_buf_set_type = typename dir_md3_rd_ctrl_policy<P>::sig_buf_set_type;
    using wn_vector_type = std::vector<wn_entry_type>;
    using blk_global_lock_type = typename dir_md3_home_ctrl_policy<P>::blk_global_lock_type;
    using inv_ctrl_type = typename dir_md3_rd_ctrl_policy<P>::inv_ctrl_type;
    using inv_ctrl_ptr_type = typename dir_md3_rd_ctrl_policy<P>::inv_ctrl_ptr_type;
    using home_ctrl_type = typename dir_md3_rd_ctrl_policy<P>::home_ctrl_type;
    using home_ctrl_ptr_type = typename dir_md3_rd_ctrl_policy<P>::home_ctrl_ptr_type;

    using rel_sig_type = typename dir_md3_sync_table_policy<P>::rel_sig_type;
    using sync_table_type = dir_sync_table<dir_md3_sync_table_policy<P>>;
    using sync_table_ptr_type = fdn::unique_ptr<sync_table_type>;
};

} // namespace medsm3
} // namespace menps

