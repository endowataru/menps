
#include <menps/medsm2/dsm/sig_table.hpp>
#include <menps/medsm2/dsm/sig_buffer.hpp>
#include <menps/medsm2/dsm/space.hpp>
#include <menps/medsm2/dsm/rel_sig.hpp>
#include <menps/medsm2/dsm/rd_set.hpp>
#include <menps/medsm2/dsm/rd_ts_state.hpp>
#include <menps/medsm2/dsm/wr_set.hpp>
#include <menps/medsm2/svm/svm_space.hpp>
#include <menps/medsm2/svm/svm_blk_table.hpp>
#include <menps/medsm2/svm/svm_seg_table.hpp>
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mecom2/rma/alltoall_buffer.hpp>
#include <menps/medsm2/dsm/queue_lock_table.hpp>
#include <menps/medsm2/dsm/fixed_lock_table.hpp>
#include <menps/medsm2/dsm/mtx_table.hpp>
#include <menps/medsm2/id_allocator.hpp>
#ifdef MEDSM2_ENABLE_REL_THREAD
#include <menps/medsm2/dsm/rel_thread.hpp>
#endif
#include <menps/medsm2/dsm/sharer_map.hpp>
#include <menps/medsm2/dsm/unordered_rd_set.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

struct dsm_base_policy
{
    using com_itf_type = dsm_com_itf_t;
    using atomic_int_type = mefdn::uint64_t;
    using size_type = mefdn::size_t;
    using ptrdiff_type = mefdn::ptrdiff_t;
    
    using ult_itf_type = typename com_itf_type::ult_itf_type;

    using blk_mutex_type = typename ult_itf_type::mutex;
    using blk_unique_lock_type =
        typename ult_itf_type::template unique_lock<blk_mutex_type>;
    
    using seg_id_type = mefdn::size_t;
    using blk_id_type = mefdn::size_t;
    using blk_pos_type = mefdn::size_t;
    
    using rd_ts_type = mefdn::uint64_t;
    using wr_ts_type = mefdn::uint64_t;
    
    using atomic_wr_ts_type = ult_itf_type::atomic<wr_ts_type>;
    
    static constexpr bool is_greater_rd_ts(const rd_ts_type a, const rd_ts_type b) noexcept {
        return a > b;
    }
    static constexpr bool is_greater_wr_ts(const wr_ts_type a, const wr_ts_type b) noexcept {
        return a > b;
    }
    static constexpr wr_ts_type make_init_wr_ts() noexcept {
        return 0;
    }
    
    using wr_set_gen_type = mefdn::ptrdiff_t;
    
    using wn_idx_type = mefdn::size_t;
    using wn_vi_type = std::vector<wn_idx_type>;
    
    template <typename Elem>
    using alltoall_buffer =
        mecom2::alltoall_buffer<
            typename com_itf_type::rma_itf_type
        ,   Elem
        >;
    template <typename Elem>
    using alltoall_ptr_set =
        mecom2::alltoall_ptr_set<
            typename com_itf_type::rma_itf_type
        ,   Elem
        >;
    
    struct wn_entry_type {
        typename com_itf_type::proc_id_type   home_proc;
        blk_id_type                 blk_id;
        rd_ts_type                  rd_ts;
        wr_ts_type                  wr_ts;
    };
    
    using wn_vector_type = std::vector<wn_entry_type>;
    
    struct constants_type {
        static const size_type max_space_size = MEDSM2_MAX_SPACE_SIZE;
        static const size_type max_seg_size = MEDSM2_MAX_SEG_SIZE;
        static const rd_ts_type lease_ts = MEDSM2_LEASE_TS;
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        static const size_type max_rel_sig_len = 0;
        #else
        static const size_type max_rel_sig_len = MEDSM2_REL_SIG_LEN;
        #endif
    };
    
    using sig_buffer_type = sig_buffer<dsm_base_policy>;

    static wr_ts_type get_wr_ts(const wn_entry_type& wn) noexcept {
        return wn.wr_ts;
    }
    
    using rel_sig_type = rel_sig<dsm_base_policy>;
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    using blk_id_set_type = std::unordered_set<blk_id_type>;
    using rd_set_type = unordered_rd_set<dsm_base_policy>;
    using rd_ts_state_type = typename rd_set_type::rd_ts_state;
    #else
    using rd_set_type = rd_set<dsm_base_policy>;
    using rd_ts_state_type = rd_ts_state<dsm_base_policy>;
    #endif
    using wr_set_type = wr_set<dsm_base_policy>;
    
    using sig_id_type = mefdn::size_t;
    
    using wr_count_type = size_type;
    static const wr_count_type wr_count_threshold = MEDSM2_WR_COUNT_THRESHOLD;
    static const wr_count_type max_fast_rel_threshold = MEDSM2_MAX_FAST_REL_THRESHOLD;
    
    static constexpr int get_tag_from_blk_id(const blk_id_type blk_id) noexcept {
        // TODO: This will probably works,
        //       but at least a magic number should be avoided.
        
        // Generate a MPI tag for the specified block.
        return static_cast<int>(blk_id >> 12);
    }
    
    using mtx_id_type = svm_space_base::mutex_id_t;
    static constexpr bool is_valid_mutex_id(const mtx_id_type mtx_id) noexcept {
        return mtx_id != 0;
    }
    
    using lock_pos_type = size_type; // TODO: define only for mtx_table
    
    using p2p_tag_type = int;
    
    #ifdef MEDSM2_ENABLE_MIGRATION
    using lock_table_type = queue_lock_table<dsm_base_policy>;
    #else
    using lock_table_type = fixed_lock_table<dsm_base_policy>;
    #endif
    using mtx_table_type = mtx_table<dsm_base_policy>;
    using id_allocator_type = id_allocator<dsm_base_policy>;
    
    static p2p_tag_type get_tag_from_lock_id(const size_type lk_id) {
        return static_cast<p2p_tag_type>(lk_id);
    }
    
    #ifdef MEDSM2_ENABLE_REL_THREAD
    using rel_thread_type = rel_thread<dsm_base_policy>;
    #endif

    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    using sharer_map_type = sharer_map<dsm_base_policy>;
    #endif
    
    using prof_aspect_type = dsm_com_policy_base::prof_aspect_type;
};

const dsm_base_policy::wr_count_type dsm_base_policy::max_fast_rel_threshold;


struct my_seg_table_policy : dsm_base_policy
{
    using derived_type = svm_seg_table<my_seg_table_policy>;
    using blk_tbl_type = svm_blk_table<dsm_base_policy>;
};
using my_seg_table = svm_seg_table<my_seg_table_policy>;


struct my_space_policy : dsm_base_policy
{
    using derived_type = svm_space<my_space_policy>;
    using seg_table_type = my_seg_table;
    using blk_tbl_type = my_seg_table_policy::blk_tbl_type;
    using sig_table_type = sig_table<dsm_base_policy>;
};

using my_space = svm_space<my_space_policy>;

struct my_space_conf {
    dsm_com_itf_t&  com;
    mefdn::size_t   max_num_sigs;
    mefdn::size_t   sig_size_in_bytes;
    mefdn::size_t   max_num_locks;
};

fdn::unique_ptr<svm_space_base> make_mpi_svm_space(dsm_com_itf_t& com)
{
    return fdn::make_unique<my_space>(
        my_space_conf{
            com
        ,   1024 // TODO: magic number
        ,   dsm_base_policy::rel_sig_type::get_max_size_in_bytes()
            // TODO: Simplify the dependencies
        ,   1024 // TODO: magic number
        }
    );
}

} // namespace medsm2
} // namespace menps

