
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/medsm2/prof.hpp>

//#define MEDSM2_FORCE_SELF_INVALIDATE_ALL

namespace menps {
namespace medsm2 {

template <typename P>
class space
{
    MEFDN_DEFINE_DERIVED(P)
    
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using seg_id_type = typename P::seg_id_type;
    using blk_id_type = typename P::blk_id_type;
    using sig_id_type = typename P::sig_id_type;
    
    using wr_ts_type = typename P::wr_ts_type;
    
    using seg_table_type = typename P::seg_table_type;
    using wr_set_type = typename P::wr_set_type;
    using rd_set_type = typename P::rd_set_type;
    using rd_ts_state_type = typename P::rd_ts_state_type;
    using rel_sig_type = typename P::rel_sig_type;
    using sig_table_type = typename P::sig_table_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    
    using size_type = typename P::size_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_unique_lock_type = typename ult_itf_type::unique_mutex_lock; // TODO
    using spinlock_type = typename ult_itf_type::spinlock;
    
    using mtx_table_type = typename P::mtx_table_type;
    using id_allocator_type = typename P::id_allocator_type;
    using mtx_id_type = typename P::mtx_id_type;
    
    #ifdef MEDSM2_ENABLE_REL_THREAD
    using rel_thread_type = typename P::rel_thread_type;
    #endif
    
public:
    template <typename Conf>
    explicit space(const Conf& conf)
        // TODO: move to coll_init()
        : seg_tbl_(conf)
    {
        sig_tbl_.coll_init(conf);
        
        mtx_tbl_.coll_init(conf);
        mtx_id_alloc_.coll_make(conf.com, conf.max_num_locks);
    }
    
    ~space()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        seg_tbl_.finalize(com);
    }
    
    void finalize()
    {
        this->wr_set_.finalize();
    }
    
    void start_release_thread()
    {
        #ifdef MEDSM2_ENABLE_REL_THREAD
        this->rel_th_.start([this] { this->fence_release(); }, MEDSM2_REL_THREAD_USEC);
        #endif
    }
    void stop_release_thread()
    {
        #ifdef MEDSM2_ENABLE_REL_THREAD
        this->rel_th_.stop();
        #endif
    }
    
    using start_read_result = typename seg_table_type::start_read_result;
    
    MEFDN_NODISCARD
    start_read_result start_read(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Read the latest block data of the home process
        // into the private area.
        // After that, the private data becomes read-only.
        return this->seg_tbl_.start_read(com, this->rd_set_, blk_id);
    }
    
    using start_write_result = typename seg_table_type::start_write_result;
    
    MEFDN_NODISCARD
    start_write_result start_write(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Copy the data of the private area to the public area,
        // and then the private area becomes writable.
        return this->seg_tbl_.start_write(com, this->rd_set_, this->wr_set_, blk_id);
    }
    
    using pin_result = typename seg_table_type::pin_result;
    
    pin_result pin(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        return this->seg_tbl_.pin(com, this->rd_set_, this->wr_set_, blk_id);
    }
    
    void unpin(const blk_id_type blk_id)
    {
        this->seg_tbl_.unpin(blk_id);
        
        this->wr_set_.add_writable(blk_id);
    }
    
    mtx_id_type allocate_mutex() {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        return this->mtx_id_alloc_.allocate(com);
    }
    void deallocate_mutex(const mtx_id_type mtx_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        this->mtx_id_alloc_.deallocate(com, mtx_id);
    }
    
    void lock_mutex(const mtx_id_type mtx_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto lk_ret = this->mtx_tbl_.lock(com, mtx_id);
        
        // Apply the WNs and self-invalidation to this process.
        this->acquire_sig(lk_ret.sig_buf);
        
        // Merge the signature for subsequent releases.
        this->rel_sig_.merge(this->seg_tbl_, lk_ret.sig_buf);
    }
    
    void unlock_mutex(const mtx_id_type mtx_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        this->fence_release();
        
        auto sig = this->rel_sig_.get_sig();
        
        this->mtx_tbl_.unlock(com, mtx_id, sig);
    }
    
    template <typename T>
    bool compare_exchange_strong_acquire(
        const sig_id_type   sig_id
    ,   const blk_id_type   blk_id
    ,   const size_type     offset
    ,   T&                  expected
    ,   const T             desired
    ) {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto cas_ret =
            this->seg_tbl_.compare_exchange(
                com, this->rd_set_, blk_id, offset, expected, desired);
        
        this->fence_acquire(sig_id);
        
        return cas_ret.is_success;
    }
    
    template <typename T>
    void store_release(
        const sig_id_type   sig_id
    ,   const blk_id_type   blk_id
    ,   const size_type     offset
    ,   const T             value
    ) {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        this->fence_release();
        
        auto sig = this->rel_sig_.get_sig();
        
        this->sig_tbl_.merge_sig_to(this->seg_tbl_, com, sig_id, sig);
        
        // Store the specified value to the atomic variable.
        this->seg_tbl_.atomic_store(com, this->rd_set_, blk_id, offset, value);
    }
    
    void fence_acquire(const sig_id_type sig_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Get the signature from the signature table.
        auto sig_buf = this->sig_tbl_.get_sig(com, sig_id);
        
        // Apply the WNs and self-invalidation to this process.
        this->acquire_sig(sig_buf);
        
        // Merge the signature for subsequent releases.
        this->rel_sig_.merge(this->seg_tbl_, sig_buf);
    }
    
    void fence_release()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto rd_ts_st = this->rd_set_.get_ts_state();
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        // Before loading the link values of blk_lock_table,
        // it is necessary to complete all the writes in this process.
        mefdn::atomic_thread_fence(mefdn::memory_order_seq_cst);
        #endif
        
        // Iterate all of the writable blocks.
        // If the callback returns false,
        // the corresponding block will be removed in the next release.
        auto wrs_ret =
            this->wr_set_.start_release_for_all_blocks(
                com, rd_ts_st, this->seg_tbl_
            );
        
        if (!wrs_ret.needs_release) {
            return;
        }
        
        // Union the write notice vector and the release signature.
        this->rel_sig_.merge(this->seg_tbl_, mefdn::move(wrs_ret.wn_vec));
        
        // Notify the other threads waiting for the finish of the release fence.
        this->wr_set_.finish_release();
    }
    
    void barrier()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        auto& coll = com.get_coll();
        
        const auto this_proc = com.this_proc_id();
        
        const auto p_all = prof::start();
        
        MEFDN_LOG_DEBUG("msg:Entering DSM barrier.");
        
        const auto num_procs = com.get_num_procs();
        
        {
            const auto p = prof::start();
            
            // Release all of the preceding writes in this thread.
            this->fence_release();
            
            prof::finish(prof_kind::fence, p);
        }
        
        const auto sig_size = this->rel_sig_.get_max_size_in_bytes();
        
        // Serialize the release signature to transfer via MPI.
        const auto this_buf = this->rel_sig_.serialize(sig_size);
        
        // Allocate an uninitialized buffer.
        const auto all_buf =
            mefdn::make_unique_uninitialized<mefdn::byte []>(sig_size * num_procs);
        
        {
            const auto p = prof::start();
            
            // Collect the signatures from all of the processes.
            coll.allgather(this_buf.get(), all_buf.get(), sig_size);
            
            prof::finish(prof_kind::barrier_allgather, p);
        }
        
        MEFDN_LOG_DEBUG("msg:Exchanged signatures for barrier.");
        
        {
            const auto p = prof::start();
            
            const auto sigs =
                mefdn::make_unique<sig_buffer_type []>(num_procs);
            
            wr_ts_type min_wr_ts = 0;
            for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
                if (proc_id != this_proc) {
                    // TODO: Remove deserialization.
                    sigs[proc_id] =
                        sig_buffer_type::deserialize_from(
                            &all_buf[sig_size * proc_id], sig_size);
                    
                    // Get the maximum of "minimum write timestamps".
                    min_wr_ts = std::max(min_wr_ts, sigs[proc_id].get_min_wr_ts());
                        // TODO: use custom comparator to avoid overflow?
                }
            }
            
            // Invalidate all of the written blocks in the write notices.
            ///*parallel*/ for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
            ult_itf_type::for_loop(
                ult_itf_type::execution::par
            ,   0
            ,   num_procs
            ,   [this, &sigs, this_proc] (const proc_id_type proc_id) {
                    if (proc_id != this_proc) {
                        this->acquire_wns(sigs[proc_id]);
                    }
                }
            );
            
            // Invalidate based on the minimum write timestamp.
            this->acquire_min_wr_ts(min_wr_ts);
            
            prof::finish(prof_kind::barrier_acq, p);
        }
        
        #ifdef MEDSM2_FORCE_SELF_INVALIDATE_ALL
        this->rd_set_.self_invalidate_all(
            [&] (const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(this->rd_set_, blk_id);
            }
        );
        #endif
        
        prof::finish(prof_kind::barrier, p_all);
        
        MEFDN_LOG_DEBUG("msg:Exiting DSM barrier.");
    }
    
private:
    void acquire_sig(const sig_buffer_type& sig)
    {
        this->acquire_wns(sig);
        
        this->acquire_min_wr_ts(sig.get_min_wr_ts());
    }
    
    void acquire_wns(const sig_buffer_type& sig)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto rd_ts_st = this->rd_set_.get_ts_state();
        
        // Invalidate the blocks based on write notices.
        sig.for_all_wns(
            [&] (const wn_entry_type& wn) {
                // Invalidate this block based on the write notice.
                // If invalidated, the home process ID and the timestamp are recorded.
                this->seg_tbl_.acquire(com, rd_ts_st, wn);
            }
        );
    }
    
    void acquire_min_wr_ts(const wr_ts_type min_wr_ts)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Self-invalidate all of the old blocks.
        // This also increases the acquire timestamp (= minimum write timestamp).
        this->rd_set_.self_invalidate(
            min_wr_ts
        ,   [&] (const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(com, rd_ts_st, blk_id);
            }
        );
    }
    
public:
    template <typename BlkTablePtr>
    void set_blk_table(const seg_id_type seg_id, BlkTablePtr&& blk_tbl_ptr) {
        // Just forward to the segment table.
        this->seg_tbl_.set_blk_table(seg_id, mefdn::forward<BlkTablePtr>(blk_tbl_ptr));
    }
    
    size_type get_blk_size(const seg_id_type seg_id) {
        // Just forward to the segment table.
        return this->seg_tbl_.get_blk_size(seg_id);
    }
    
    size_type get_blk_pos(const seg_id_type seg_id, const blk_id_type blk_id) {
        // Just forward to the segment table.
        return this->seg_tbl_.get_blk_pos(seg_id, blk_id);
    }
    
private:
    seg_table_type  seg_tbl_;
    
    wr_set_type     wr_set_;
    
    rd_set_type     rd_set_;
    
    rel_sig_type    rel_sig_;
    
    sig_table_type  sig_tbl_;
    
    mtx_table_type  mtx_tbl_;
    
    id_allocator_type   mtx_id_alloc_;
    
    #ifdef MEDSM2_ENABLE_REL_THREAD
    rel_thread_type rel_th_;
    #endif
};

} // namespace medsm2
} // namespace menps
