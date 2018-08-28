
#include <menps/medsm2/sig_table.hpp>
#include <menps/medsm2/sig_buffer.hpp>
#include <menps/medsm2/space.hpp>
#include <menps/medsm2/rel_sig.hpp>
#include <menps/medsm2/rd_set.hpp>
#include <menps/medsm2/wr_set.hpp>
#include <menps/medsm2/svm/svm_space.hpp>
#include <menps/medsm2/svm/svm_blk_table.hpp>
#include <menps/medsm2/svm/svm_seg_table.hpp>
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/p2p/mpi/mpi_p2p.hpp>
#include <menps/mecom2/rma/alltoall_buffer.hpp>

#ifdef MEOMP_SEPARATE_WORKER_THREAD
#include <menps/meult/klt.hpp>
#endif

namespace menps {
namespace medsm2 {

using mecom2::mpi_coll;
using mecom2::mpi_p2p;

class mpi_com_itf
{
public:
    using rma_itf_type = mpi_svm_rma_type;
    using coll_itf_type = mpi_coll;
    using p2p_itf_type = mpi_p2p;
    
    template <typename Conf>
    explicit mpi_com_itf(const Conf& conf)
        : rma_(conf.rma)
        , coll_(conf.coll)
        , p2p_(conf.p2p)
    {
        this->proc_id_ = get_coll().this_proc_id();
        this->num_procs_ = get_coll().get_num_procs();
    }
    
    using proc_id_type = int;
    
    int this_proc_id() { return proc_id_; }
    int get_num_procs() { return num_procs_; }
    
    rma_itf_type& get_rma() { return rma_; }
    mpi_coll& get_coll() { return coll_; }
    mpi_p2p& get_p2p() { return this->p2p_; }
    
private:
    rma_itf_type& rma_;
    coll_itf_type& coll_;
    p2p_itf_type& p2p_;
    int proc_id_;
    int num_procs_;
};

struct dsm_base_policy
{
    using com_itf_type = mpi_com_itf;
    using atomic_int_type = mefdn::uint64_t;
    using size_type = mefdn::size_t;
    using ptrdiff_type = mefdn::ptrdiff_t;
    
    using seg_id_type = mefdn::size_t;
    using blk_id_type = mefdn::size_t;
    using blk_pos_type = mefdn::size_t;
    
    using rd_ts_type = mefdn::uint64_t;
    using wr_ts_type = mefdn::uint64_t;
    
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
    using wn_vi_type = mefdn::vector<wn_idx_type>;
    
    using ult_itf_type = medsm2::default_ult_itf;
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    using worker_ult_itf_type = meult::klt_policy;
    #else
    using worker_ult_itf_type = medsm2::default_ult_itf;
    #endif
    
    #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    using mutex_type = mefdn::spinlock;
    using unique_lock_type = mefdn::unique_lock<mefdn::spinlock>;
    #else
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::unique_mutex_lock; // TODO: rename
    #endif
    
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
        mpi_com_itf::proc_id_type   home_proc;
        blk_id_type                 blk_id;
        rd_ts_type                  rd_ts;
        wr_ts_type                  wr_ts;
    };
    
    using wn_vector_type = mefdn::vector<wn_entry_type>;
    
    struct constants_type {
        static const size_type max_space_size = MEDSM2_MAX_SPACE_SIZE;
        static const size_type max_seg_size = MEDSM2_MAX_SEG_SIZE;
        static const rd_ts_type lease_ts = MEDSM2_LEASE_TS;
        static const size_type max_rel_sig_len = MEDSM2_REL_SIG_LEN;
    };
    
    using sig_buffer_type = sig_buffer<dsm_base_policy>;
    
    using rel_sig_type = rel_sig<dsm_base_policy>;
    using rd_set_type = rd_set<dsm_base_policy>;
    using wr_set_type = wr_set<dsm_base_policy>;
    
    using sig_id_type = mefdn::size_t;
    
    using wr_count_type = size_type;
    static const wr_count_type wr_count_threshold = MEDSM2_WR_COUNT_THRESHOLD;
    
    static constexpr int get_tag_from_blk_id(const blk_id_type blk_id) noexcept {
        // TODO: This will probably works,
        //       but at least a magic number should be avoided.
        
        // Generate a MPI tag for the specified block.
        return static_cast<int>(blk_id >> 12);
    }
};


struct my_blk_table_policy : dsm_base_policy
{
};

struct my_seg_table_policy : my_blk_table_policy
{
    using derived_type = svm_seg_table<my_seg_table_policy>;
    using blk_tbl_type = svm_blk_table<my_blk_table_policy>;
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


class mpi_svm_space::impl
{
    struct com_conf {
        mpi_svm_rma_type&   rma;
        mpi_coll&           coll;
        mpi_p2p&            p2p;
    };
    
    struct my_space_conf {
        mpi_com_itf&    com;
        mefdn::size_t   max_num_sigs;
        mefdn::size_t   sig_size_in_bytes;
    };
public:
    explicit impl(
        mpi_svm_rma_type&   rma
    ,   mecom2::mpi_coll&   coll
    ,   mecom2::mpi_p2p&    p2p
    )
        : com_(com_conf{ rma, coll, p2p })
        , sp_(
            my_space_conf{
                com_
            ,   1024 // TODO: magic number
            ,   dsm_base_policy::rel_sig_type::get_max_size_in_bytes()
                // TODO: Simplify the dependencies
            }
        )
    {
        //this->sp_.coll_init(my_space_conf{ com_, 1<<15, 1<<10 }); // TODO: magic numbers
    }
    
    my_space& space() { return sp_; }
    
private:
    mpi_com_itf com_;
    my_space sp_;
};

mpi_svm_space::mpi_svm_space(
    mpi_svm_rma_type&   rma
,   mecom2::mpi_coll&   coll
,   mecom2::mpi_p2p&    p2p
)
    : impl_(new impl(rma, coll, p2p ))
{ }

mpi_svm_space::~mpi_svm_space() = default;

void* mpi_svm_space::coll_alloc_seg(const size_type seg_size, const size_type blk_size) {
    return this->impl_->space().coll_alloc_seg(seg_size, blk_size);
}

void mpi_svm_space::coll_alloc_global_var_seg(const size_type seg_size, const size_type blk_size, void* const start_ptr)
{
    return this->impl_->space().coll_alloc_global_var_seg(seg_size, blk_size, start_ptr);
}

bool mpi_svm_space::compare_exchange_strong_acquire(
    mefdn::uint32_t&        target
,   mefdn::uint32_t&        expected
,   const mefdn::uint32_t   desired
) {
    return this->impl_->space().compare_exchange_strong_acquire(target, expected, desired);
}

void mpi_svm_space::store_release(
    mefdn::uint32_t* const  ptr
,   const mefdn::uint32_t   val
) {
    this->impl_->space().store_release(*ptr, val);
}

#if 0
mefdn::uint64_t mpi_svm_space::load_acquire(mefdn::uint64_t* const ptr)
{
    return this->impl_->space().load_acquire(ptr);
}

#endif
void mpi_svm_space::barrier() {
    this->impl_->space().barrier();
}

void mpi_svm_space::pin(void* const ptr, const size_type size)
{
    this->impl_->space().pin(ptr, size);
}

void mpi_svm_space::unpin(void* const ptr, const size_type size)
{
    this->impl_->space().unpin(ptr, size);
}

void mpi_svm_space::enable_on_this_thread()
{
    this->impl_->space().enable_on_this_thread();
}
void mpi_svm_space::disable_on_this_thread()
{
    this->impl_->space().disable_on_this_thread();
}

bool mpi_svm_space::try_upgrade(void* const ptr)
{
    return this->impl_->space().try_upgrade(ptr);
}

} // namespace medsm2
} // namespace menps

