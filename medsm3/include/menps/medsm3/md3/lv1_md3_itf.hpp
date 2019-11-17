
#pragma once

#include <menps/medsm3/svm/svm_segment.hpp>
#include <menps/medsm3/svm/svm_segment_set.hpp>
#include <menps/medsm3/layout/blk_local_lock.hpp>
#include <menps/medsm3/layout/blk_global_lock_base.hpp>
#include <menps/medsm3/reader/ts/ts_segment.hpp>
#include <menps/medsm2/dsm/queue_lock_table.hpp>
#include <menps/mefdn/memory/mapped_memory.hpp>
#include <menps/medsm2/svm/shm_object.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
struct lv1_md3_policy_base
{
    using com_itf_type = typename P::com_itf_type;
    using ult_itf_type = typename com_itf_type::ult_itf_type;
    using mtx_id_type = typename P::mtx_id_type;

    using seg_id_type = fdn::size_t;
    using blk_subindex_type = fdn::size_t;

    struct blk_id_type {
        seg_id_type         seg_id;
        blk_subindex_type   sidx;

        bool operator < (const blk_id_type& other) const noexcept {
            return this->seg_id < other.seg_id
                || (this->seg_id == other.seg_id && this->sidx < other.sidx);
        }
        bool operator == (const blk_id_type& other) const noexcept {
            return this->seg_id == other.seg_id && this->sidx == other.sidx;
        }

        std::string to_str() const {
            fmt::memory_buffer w;
            format_to(w, "{},{}", this->seg_id, this->sidx);
            return to_string(w);
        }
    };
    static bool is_valid_blk_id(const blk_id_type& blk_id) noexcept {
        return blk_id.seg_id != 0 || blk_id.sidx != 0;
    }

    using ts_type = fdn::uint64_t;
    using wr_ts_type = ts_type;
    using rd_ts_type = ts_type;

    using lock_tag_type = int;  // TODO: comes from p2p_itf_type

    using blk_mutex_type = typename ult_itf_type::mutex;

    using blk_local_lock_type = blk_local_lock<lv1_md3_policy_base<P>>;
    using blk_global_lock_base_type = blk_global_lock_base<lv1_md3_policy_base<P>>;
    
    using assert_policy_type = typename ult_itf_type::assert_policy;
    using log_aspect_type = typename P::log_aspect_type;

    struct ts_interval_type {
        ts_type wr_ts;
        ts_type rd_ts;
    };

    using constants_type = typename P::constants_type;
};

enum class md3_blk_state_t {
    invalid_clean = 0
,   invalid_dirty
,   readonly_clean
,   readonly_dirty
,   writable
,   pinned   // special state for call stacks
};

template <typename P>
struct md3_global_lock_table_policy
{
    using com_itf_type = typename lv1_md3_policy_base<P>::com_itf_type;
    using lock_pos_type = typename lv1_md3_policy_base<P>::blk_subindex_type;
    using p2p_tag_type = typename lv1_md3_policy_base<P>::lock_tag_type;
    using atomic_int_type = mefdn::uint64_t; // TODO: comes from rma_itf_type
    using size_type = fdn::size_t;

    template <typename T>
    using alltoall_buffer = typename com_itf_type::template alltoall_buffer_t<T>;

    using prof_aspect_type = typename P::prof_aspect_type;
};

template <typename P>
struct md3_segment_policy
    : lv1_md3_policy_base<P>
{
    using segment_base_type = ts_segment<md3_segment_policy>;

    using global_lock_table_type = medsm2::queue_lock_table<md3_global_lock_table_policy<P>>;
    using blk_state_type = md3_blk_state_t;
    using wr_count_type = fdn::size_t;
    using mapped_memory_type = fdn::mapped_memory;
};

template <typename P>
struct mv3_segment_set_policy
    : lv1_md3_policy_base<P>
{
    using derived_type = svm_segment_set<mv3_segment_set_policy>;
    using segment_type = svm_segment<md3_segment_policy<P>>;
    using shm_object_type = medsm2::shm_object;
};


template <typename P>
struct lv1_md3_itf
{
    using segment_set_type = svm_segment_set<mv3_segment_set_policy<P>>;
    using segment_set_ptr_type = std::shared_ptr<segment_set_type>;
    using segment_type = svm_segment<md3_segment_policy<P>>;

    using blk_id_type = typename lv1_md3_policy_base<P>::blk_id_type;
    using mtx_id_type = typename lv1_md3_policy_base<P>::mtx_id_type;
    using com_itf_type = typename lv1_md3_policy_base<P>::com_itf_type;
    using ult_itf_type = typename com_itf_type::ult_itf_type;

    using ts_type = typename lv1_md3_policy_base<P>::ts_type;
    using wr_ts_type = typename lv1_md3_policy_base<P>::wr_ts_type;
    using rd_ts_type = typename lv1_md3_policy_base<P>::rd_ts_type;
    using ts_interval_type = typename lv1_md3_policy_base<P>::ts_interval_type;

    using blk_local_lock_type = typename lv1_md3_policy_base<P>::blk_local_lock_type;
    using blk_global_lock_base_type =
        typename lv1_md3_policy_base<P>::blk_global_lock_base_type;

    using blk_state_type = typename md3_segment_policy<P>::blk_state_type;
    using wr_count_type = typename md3_segment_policy<P>::wr_count_type;
    using blk_mutex_type = typename lv1_md3_policy_base<P>::blk_mutex_type;
     
    using prof_aspect_type = typename P::prof_aspect_type;

    using constants_type = typename lv1_md3_policy_base<P>::constants_type;
    
    // Used by mtx_table.
    // TODO: Separate into two implementations.
    using global_lock_table_type = typename md3_segment_policy<P>::global_lock_table_type;
};

} // namespace medsm3
} // namespace menps

