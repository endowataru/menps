
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/medsm2/svm/svm_space_base.hpp>

//#define MEDSM2_FORCE_SELF_INVALIDATE_ALL

namespace menps {
namespace medsm2 {

template <typename P>
class space
    : public svm_space_base
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
        
        auto& coll = com.get_coll();
        const auto this_proc = com.this_proc_id();
        const auto num_procs = com.get_num_procs();
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            if (proc == this_proc) {
                P::prof_aspect_type::print_all("medsm2", this_proc);
            }
            coll.barrier();
        }
    }
    
    void finalize()
    {
        this->wr_set_.finalize();
    }
    
    virtual void start_release_thread() override
    {
        #ifdef MEDSM2_ENABLE_REL_THREAD
        this->rel_th_.start([this] { this->fence_release(); }, MEDSM2_REL_THREAD_USEC);
        #endif
    }
    virtual void stop_release_thread() override
    {
        #ifdef MEDSM2_ENABLE_REL_THREAD
        this->rel_th_.stop();
        #endif
    }
    
    using start_read_result = typename seg_table_type::start_read_result;
    
    MEFDN_NODISCARD
    start_read_result start_read(const blk_id_type blk_id)
    {
        CMPTH_P_PROF_SCOPE(P, read_upgrade);
        
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
        CMPTH_P_PROF_SCOPE(P, write_upgrade);
        
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Copy the data of the private area to the public area,
        // and then the private area becomes writable.
        return this->seg_tbl_.start_write(com, this->rd_set_, this->wr_set_, blk_id);
    }
    
    using pin_result = typename seg_table_type::pin_result;
    
    pin_result pin_block(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        return this->seg_tbl_.pin(com, this->rd_set_, this->wr_set_, blk_id);
    }
    
    void unpin_block(const blk_id_type blk_id)
    {
        this->seg_tbl_.unpin(blk_id);
        
        this->wr_set_.add_writable(blk_id);
    }
    
    virtual mtx_id_type allocate_mutex() override
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        return this->mtx_id_alloc_.allocate(com);
    }
    virtual void deallocate_mutex(const mtx_id_type mtx_id) override
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        this->mtx_id_alloc_.deallocate(com, mtx_id);
    }
    
    virtual void lock_mutex(const mtx_id_type mtx_id) override
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto lk_ret = this->mtx_tbl_.lock(com, mtx_id);
        
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        this->fence_acquire();
        #else
        // Apply the WNs and self-invalidation to this process.
        this->acquire_sig(lk_ret.sig_buf);
        
        // Merge the signature for subsequent releases.
        this->rel_sig_.merge(this->seg_tbl_, lk_ret.sig_buf);
        #endif
    }
    
    virtual void unlock_mutex(const mtx_id_type mtx_id) override
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        this->fence_release();
        
        auto sig = this->rel_sig_.get_sig();
        
        this->mtx_tbl_.unlock(com, mtx_id, sig);
    }
    
    template <typename T>
    bool compare_exchange_strong_acquire_general(
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
    void store_release_general(
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
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        this->fence_acquire();
        
        #else
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Get the signature from the signature table.
        auto sig_buf = this->sig_tbl_.get_sig(com, sig_id);
        
        // Apply the WNs and self-invalidation to this process.
        this->acquire_sig(sig_buf);
        
        // Merge the signature for subsequent releases.
        this->rel_sig_.merge(this->seg_tbl_, sig_buf);
        #endif
    }
    
    void fence_release()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        CMPTH_P_PROF_SCOPE(P, fence_release);
        
        const auto rd_ts_st = this->rd_set_.get_ts_state();
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        // Before loading the link values of blk_lock_table,
        // it is necessary to complete all the writes in this process.
        ult_itf_type::atomic_thread_fence(ult_itf_type::memory_order_seq_cst);
        #endif
        
        // Iterate all of the writable blocks.
        // If the callback returns false,
        // the corresponding block will be removed in the next release.
        auto wrs_ret =
            this->wr_set_.start_release_for_all_blocks(
                rd_ts_st
            ,   [&com, this] (const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id) {
                    return this->seg_tbl_.release(com, rd_ts_st, blk_id);
                }
            ,   convert_to_wn_vec{self}
            );
        
        if (!wrs_ret.needs_release) {
            return;
        }
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        // Union the write notice vector and the release signature.
        this->rel_sig_.merge(this->seg_tbl_, mefdn::move(wrs_ret.wn_vec));
        #endif
        
        // Notify the other threads waiting for the finish of the release fence.
        this->wr_set_.finish_release();
    }
    
private:
    struct convert_to_wn_vec {
        derived_type&   self;
        template <typename BlkIdIter, typename RelRetIter>
        wn_vector_type operator() (BlkIdIter blk_id_first, BlkIdIter blk_id_last, RelRetIter rel_ret_first) {
            wn_vector_type wn_vec;
            
            #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
            const auto num_released =
                static_cast<size_type>(blk_id_last - blk_id_first);
            // Pre-allocate the write notice vector.
            wn_vec.reserve(num_released);
            
            // Check all of the release results sequentially.
            for (size_type i = 0; i < num_released; ++i) {
                const auto blk_id = *(blk_id_first+i);
                const auto& rel_ret = *(rel_ret_first+i);
                
                if (rel_ret.release_completed && rel_ret.is_written)
                {
                    // Add to the write notices
                    // because the current process modified this block.
                    wn_vec.push_back(wn_entry_type{
                        rel_ret.new_owner, blk_id, rel_ret.new_rd_ts, rel_ret.new_wr_ts
                    });
                }
            }
            #endif
            
            return wn_vec;
        }
    };
    
public:
    virtual void barrier() override
    {
        CMPTH_P_PROF_SCOPE(P, barrier);
        
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        auto& coll = com.get_coll();
        
        const auto this_proc = com.this_proc_id();
        const auto num_procs = com.get_num_procs();
        
        MEFDN_LOG_DEBUG("msg:Entering DSM barrier.");
        
        {
            CMPTH_P_PROF_SCOPE(P, barrier_release);
            
            // Release all of the preceding writes in this thread.
            this->fence_release();
        }
        
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        coll.barrier();
        
        {
            CMPTH_P_PROF_SCOPE(P, barrier_acquire);
            this->fence_acquire();
        }
        
        #else
        const auto sig_size = this->rel_sig_.get_max_size_in_bytes();
        
        // Serialize the release signature to transfer via MPI.
        const auto this_buf = this->rel_sig_.serialize(sig_size);
        
        // Allocate an uninitialized buffer.
        const auto all_buf =
            mefdn::make_unique_uninitialized<mefdn::byte []>(sig_size * num_procs);
        
        {
            CMPTH_P_PROF_SCOPE(P, barrier_allgather);
            
            // Collect the signatures from all of the processes.
            coll.allgather(this_buf.get(), all_buf.get(), sig_size);
        }
        
        MEFDN_LOG_DEBUG("msg:Exchanged signatures for barrier.");

        {
            CMPTH_P_PROF_SCOPE(P, barrier_acquire);
            
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
        }
        #endif
        
        #ifdef MEDSM2_FORCE_SELF_INVALIDATE_ALL
        this->rd_set_.self_invalidate_all(
            [&] (const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(this->rd_set_, blk_id);
            }
        );
        #endif
        
        MEFDN_LOG_DEBUG("msg:Exiting DSM barrier.");
    }
    
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
private:
    void fence_acquire()
    {
        MEFDN_LOG_VERBOSE("msg:Start acquire fence.");
        
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        this->rd_set_.self_invalidate(
            [&] (const rd_ts_state_type& rd_ts_st, const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(com, rd_ts_st, blk_id);
            }
        );
        
        MEFDN_LOG_VERBOSE("msg:Finish acquire fence.");
    }
    
    #else // MEDSM2_USE_DIRECTORY_COHERENCE
private:
    void acquire_sig(const sig_buffer_type& sig)
    {
        MEFDN_LOG_VERBOSE("msg:Start acquiring signature.");
        
        this->acquire_wns(sig);
        
        this->acquire_min_wr_ts(sig.get_min_wr_ts());
        
        MEFDN_LOG_VERBOSE("msg:Finish acquiring signature.");
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
    #endif
    
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

    virtual void enable_prof() noexcept override {
        P::prof_aspect_type::set_enabled(true);
    }
    virtual void disable_prof() noexcept override {
        P::prof_aspect_type::set_enabled(false);
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

