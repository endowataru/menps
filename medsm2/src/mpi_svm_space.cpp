
#include <menps/medsm2/sig_buffer.hpp>
#include <menps/medsm2/space.hpp>
#include <menps/medsm2/rel_sig.hpp>
#include <menps/medsm2/acq_sig.hpp>
#include <menps/medsm2/rd_set.hpp>
#include <menps/medsm2/wr_set.hpp>
#include <menps/medsm2/inplace_wr_set.hpp>
#include <menps/medsm2/svm/svm_space.hpp>
#include <menps/medsm2/svm/svm_blk_table.hpp>
#include <menps/medsm2/svm/svm_seg_table.hpp>
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/rma/mpi/mpi_alltoall_buffer.hpp>
#include <menps/mecom2/rma/mpi/mpi_alltoall_ptr_set.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>

namespace menps {
namespace medsm2 {

using mecom2::mpi_rma;
using mecom2::mpi_coll;
using mecom2::mpi_alltoall_buffer;
using mecom2::mpi_alltoall_ptr_set;

class mpi_com_itf
{
public:
    using rma_itf_type = mpi_rma;
    using coll_itf_type = mpi_coll;
    
    template <typename Conf>
    explicit mpi_com_itf(const Conf& conf)
        : rma_(conf.rma)
        , coll_(conf.coll)
    {
        this->proc_id_ = get_coll().current_process_id();
        this->num_procs_ = get_coll().number_of_processes();
    }
    
    using proc_id_type = int;
    
    int this_proc_id() { return proc_id_; }
    int get_num_procs() { return num_procs_; }
    
    mpi_rma& get_rma() { return rma_; }
    mpi_coll& get_coll() { return coll_; }
    
private:
    rma_itf_type& rma_;
    coll_itf_type& coll_;
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
    
    #ifdef MEDSM2_USE_INPLACE_WR_SET
    using rel_pos_type = mefdn::ptrdiff_t;
    #else
    using rel_pos_type = mefdn::size_t;
    #endif
    
    using wn_idx_type = mefdn::size_t;
    using wn_vi_type = mefdn::vector<wn_idx_type>;
    
    using mutex_type = mefdn::mutex;
    using mutex_unique_lock_type = mefdn::unique_lock<mefdn::mutex>;
    using cv_type = mefdn::condition_variable;
    using unique_lock_type = mefdn::unique_lock<mefdn::mutex>;
    using thread_type = mefdn::thread;
    
    template <typename T>
    using alltoall_buffer = mpi_alltoall_buffer<T>;
    
    template <typename T>
    using alltoall_ptr_set = mpi_alltoall_ptr_set<T>;
    
    struct wn_entry_type {
        mpi_com_itf::proc_id_type   home_proc;
        blk_id_type                 blk_id;
        rd_ts_type                  rd_ts;
        wr_ts_type                  wr_ts;
    };
    
    using wn_vector_type = mefdn::vector<wn_entry_type>;
    
    struct constants_type {
        static const mefdn::size_t max_space_size = 0x100000000000; // TODO
        static const mefdn::size_t max_seg_size = max_space_size / 4096; // TODO
        static const rd_ts_type lease_ts = 10; // TODO
        static const mefdn::size_t max_rel_sig_len = 1024;
    };
    
    using sig_buffer_type = sig_buffer<dsm_base_policy>;
    
    using rel_sig_type = rel_sig<dsm_base_policy>;
    using acq_sig_type = acq_sig<dsm_base_policy>;
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
    #ifdef MEDSM2_USE_INPLACE_WR_SET
    using wr_set_type = inplace_wr_set<dsm_base_policy>;
    #else
    using wr_set_type = wr_set<dsm_base_policy>;
    #endif
    using rd_set_type = rd_set<dsm_base_policy>;
};

using my_space = svm_space<my_space_policy>;


class mpi_svm_space::impl
{
    struct com_conf {
        mpi_rma&    rma;
        mpi_coll&   coll;
    };
    
    struct my_space_conf {
        mpi_com_itf&    com;
    };
public:
    explicit impl(
        mecom2::mpi_rma&    rma
    ,   mecom2::mpi_coll&   coll
    )
        : com_(com_conf{ rma, coll })
        , sp_(my_space_conf{ com_ }) // TODO: magic numbers
    {
        //this->sp_.coll_init(my_space_conf{ com_, 1<<15, 1<<10 }); // TODO: magic numbers
    }
    
    my_space& space() { return sp_; }
    
private:
    mpi_com_itf com_;
    my_space sp_;
};

mpi_svm_space::mpi_svm_space(
    mecom2::mpi_rma&    rma
,   mecom2::mpi_coll&   coll
)
    : impl_(new impl(rma, coll))
{ }

mpi_svm_space::~mpi_svm_space() = default;

void* mpi_svm_space::coll_alloc_seg(const size_type seg_size, const size_type blk_size) {
    return this->impl_->space().coll_alloc_seg(seg_size, blk_size);
}

void mpi_svm_space::coll_alloc_global_var_seg(const size_type seg_size, const size_type blk_size, void* const start_ptr)
{
    return this->impl_->space().coll_alloc_global_var_seg(seg_size, blk_size, start_ptr);
}
#if 0
void mpi_svm_space::store_release(mefdn::uint64_t* const ptr, mefdn::uint64_t val)
{
    this->impl_->space().store_release(ptr, val);
}

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

} // namespace medsm2
} // namespace menps

