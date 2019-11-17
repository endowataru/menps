
#pragma once

#include <menps/medsm3/svm/basic_svm_space.hpp>
#include <menps/medsm3/reader/ts/ts_sync_table.hpp>
#include <menps/medsm2/id_allocator.hpp>
#include <menps/medsm2/dsm/mtx_table.hpp>

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
    using log_aspect_type = typename ult_itf_type::log_aspect;

    using prof_aspect_type = typename lv4_itf_type::prof_aspect_type;

    using blk_id_type = typename lv4_itf_type::blk_id_type;
};

template <typename P>
struct md3_mtx_table_policy
    : lv5_md3_policy_base<P>
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;

public:
    using sig_buffer_type = typename lv4_itf_type::sig_buffer_type;
    using lock_table_type = typename lv4_itf_type::global_lock_table_type;

    using mtx_id_type = typename lv4_itf_type::mtx_id_type;
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
struct md3_id_allocator_policy
    : lv5_md3_policy_base<P>
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;

public:
    using size_type = fdn::size_t;
    using atomic_int_type = fdn::uint64_t; // TODO
};

template <typename P>
struct md3_sync_table_policy
    : lv5_md3_policy_base<P>
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;

public:
    using sig_buffer_type = typename lv4_itf_type::sig_buffer_type;
    using sig_buf_set_type = typename lv4_itf_type::sig_buf_set_type;
    using mtx_id_type = typename lv4_itf_type::mtx_id_type;
    using mtx_table_type = medsm2::mtx_table<md3_mtx_table_policy<P>>;
    using prof_aspect_type = typename lv4_itf_type::prof_aspect_type;
    using rel_sig_type = typename lv4_itf_type::rel_sig_type;

    using id_allocator_type = medsm2::id_allocator<md3_id_allocator_policy<P>>;
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

    using sync_table_type = ts_sync_table<md3_sync_table_policy<P>>;
    using sync_table_ptr_type = fdn::unique_ptr<sync_table_type>;

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

