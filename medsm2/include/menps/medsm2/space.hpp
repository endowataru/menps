
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>

//#define MEDSM2_FORCE_SELF_INVALIDATE_ALL
#define MEDSM2_USE_INPLACE_WR_SET

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
    
    using seg_table_type = typename P::seg_table_type;
    using wr_set_type = typename P::wr_set_type;
    using rd_set_type = typename P::rd_set_type;
    using rel_sig_type = typename P::rel_sig_type;
    using acq_sig_type = typename P::acq_sig_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    
    using size_type = typename P::size_type;
    
public:
    template <typename Conf>
    explicit space(const Conf& conf)
        // TODO: move to coll_init()
        : seg_tbl_(conf)
    { }
    
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
    
    using start_read_result = typename seg_table_type::start_read_result;
    
    MEFDN_NODISCARD
    start_read_result start_read(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Read the latest block data of the home process
        // into the private area.
        // After that, the private data becomes read-only.
        const auto ret =
            this->seg_tbl_.start_read(com, this->acq_sig_, blk_id);
        
        this->after_read(blk_id, ret);
        
        return ret;
    }
    
    using start_write_result = typename seg_table_type::start_write_result;
    
    MEFDN_NODISCARD
    start_write_result start_write(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        // Copy the data of the private area to the public area,
        // and then the private area becomes writable.
        const auto ret =
            this->seg_tbl_.start_write(com, this->acq_sig_, blk_id);
        
        this->after_write(blk_id, ret);
        
        return ret;
    }
    
    using pin_result = typename seg_table_type::pin_result;
    
    pin_result pin(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto ret =
            this->seg_tbl_.pin(com, this->acq_sig_, blk_id);
        
        this->after_write(blk_id, ret);
        
        return ret;
    }
    
private:
    void after_read(const blk_id_type blk_id, const start_read_result& ret)
    {
        if (ret.is_newly_read) {
            // Add this block to the read set.
            // TODO: Review concurrent accesses to this block.
            this->rd_set_.add_readable(blk_id, ret.rd_ts);
        }
    }
    
    void after_write(const blk_id_type blk_id, const start_write_result& ret)
    {
        this->after_read(blk_id, ret.read);
        
        if (ret.write.needs_protect) {
            // Add to the write set.
            // This MUST FOLLOW the mprotect(PROT_READ|PROT_WRITE) call
            // because the write set may be flushed in the reversed order.
            this->wr_set_.add_writable(blk_id);
        }
    }
    
public:
    void unpin(const blk_id_type blk_id)
    {
        this->seg_tbl_.unpin(blk_id);
        
        this->wr_set_.add_writable(blk_id);
    }
    
public:
    #ifdef MEDSM2_USE_INPLACE_WR_SET
    void fence()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto this_proc = com.this_proc_id();
        
        wn_vector_type wn_vec;
        mefdn::vector<blk_id_type> clean_ids;
        
        // Iterate all of the writable blocks.
        // If the callback returns false,
        // the corresponding block will be removed in the next release.
        const auto wrs_ret =
            this->wr_set_.start_release_for_all_blocks(
                [&] (const blk_id_type blk_id) {
                    // Write the data to the public area
                    // and migrate from the old owner if necessary.
                    // TODO: Make this a coroutine.
                    const auto rel_ret =
                        this->seg_tbl_.release(com, this->acq_sig_, blk_id, false);
                    
                    if (rel_ret.release_completed) {
                        // This block was not marked as "released"
                        // and the release operation for it has completed.
                        
                        if (rel_ret.is_written) {
                            // Add to the write notices
                            // because the current process modified this block.
                            wn_vec.push_back(wn_entry_type{ this_proc, blk_id, rel_ret.new_rd_ts, rel_ret.new_wr_ts });
                        }
                        
                        // Return whether this block became inaccessible in this release.
                        if (rel_ret.is_clean) {
                            // If this block became inaccessible now, the subsequent release operations
                            // need not to track this block (if it isn't added to the write set again).
                            clean_ids.push_back(blk_id);
                        }
                    }
                    else {
                        // There are two cases:
                        // (1) This block is pinned.
                        // (2) This block was store-released and released later.
                        // If this block ID is not in "ordered_ids" but marked as "released",
                        // it will be released in the next release (not in this time).
                    }
                }
            );
        
        if (!wrs_ret.needs_release) {
            return;
        }
        
        // Union the write notice vector and the release signature.
        this->rel_sig_.merge(mefdn::move(wn_vec));
        
        // Remove the block IDs that were downgraded during this release.
        this->wr_set_.remove(mefdn::begin(clean_ids), mefdn::end(clean_ids));
        
        // TODO: Atomics are not implemented...
        
        this->wr_set_.finish_release();
    }
    
    bool progress_release() { return false; }
    
    #else
    MEFDN_NODISCARD
    bool progress_release()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        
        const auto this_proc = com.this_proc_id();
        
        wn_vector_type wn_vec;
        mefdn::vector<blk_id_type> clean_ids;
        
        // Iterate all of the writable blocks.
        // If the callback returns false,
        // the corresponding block will be removed in the next release.
        const auto wrs_ret =
            this->wr_set_.release_for_all_blocks(
                [&] (const blk_id_type blk_id) {
                    // Write the data to the public area
                    // and migrate from the old owner if necessary.
                    // TODO: Make this a coroutine.
                    const auto rel_ret =
                        this->seg_tbl_.release(com, blk_id, false);
                    
                    if (rel_ret.release_completed) {
                        // This block was not marked as "released"
                        // and the release operation for it has completed.
                        
                        if (rel_ret.is_written) {
                            // Add to the write notices
                            // because the current process modified this block.
                            wn_vec.push_back(wn_entry_type{ this_proc, blk_id, rel_ret.new_rd_ts, rel_ret.new_wr_ts });
                        }
                        
                        // Return whether this block became inaccessible in this release.
                        if (rel_ret.is_clean) {
                            // If this block became inaccessible now, the subsequent release operations
                            // need not to track this block (if it isn't added to the write set again).
                            clean_ids.push_back(blk_id);
                        }
                    }
                    else {
                        // There are two cases:
                        // (1) This block is pinned.
                        // (2) This block was store-released and released later.
                        // If this block ID is not in "ordered_ids" but marked as "released",
                        // it will be released in the next release (not in this time).
                    }
                }
            );
        
        if (wrs_ret.finished) {
            return false;
        }
        
        // Union the write notice vector and the release signature.
        this->rel_sig_.merge(mefdn::move(wn_vec));
        
        // Remove the block IDs that were downgraded during this release.
        this->wr_set_.remove(mefdn::begin(clean_ids), mefdn::end(clean_ids));
        
        // TODO: Atomics are not implemented...
        
        return true;
    }
    #endif
    
    void barrier()
    {
        auto& self = this->derived();
        auto& com = self.get_com_itf();
        auto& coll = com.get_coll();
        
        const auto this_proc = com.this_proc_id();
        
        MEFDN_LOG_DEBUG("msg:Entering DSM barrier.");
        
        const auto num_procs = com.get_num_procs();
        
        // Complete all of the preceding writes in the releaser thread.
        // After releasing all, the releaser thread wakes up this thread.
        #ifdef MEDSM2_USE_INPLACE_WR_SET
        this->fence();
        #else
        this->wr_set_.fence();
        #endif
        
        const auto sig_size = this->rel_sig_.get_max_size_in_bytes();
        
        // Serialize the release signature to transfer via MPI.
        const auto this_buf = this->rel_sig_.serialize(sig_size);
        
        // Allocate an uninitialized buffer.
        const auto all_buf =
            mefdn::make_unique_uninitialized<mefdn::byte []>(sig_size * num_procs);
        
        // Collect the signatures from all of the processes.
        coll.allgather(this_buf.get(), all_buf.get(), sig_size);
        
        MEFDN_LOG_DEBUG("msg:Exchanged signatures for barrier.");
        
        // Invalidate all of the written blocks in the write notices.
        /*parallel*/ for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
            if (proc_id != this_proc) {
                // TODO: Remove deserialization.
                auto sig = sig_buffer_type::deserialize_from(&all_buf[sig_size * proc_id], sig_size);
                
                this->acquire_sig(sig);
            }
        }
        
        #ifdef MEDSM2_FORCE_SELF_INVALIDATE_ALL
        this->rd_set_.self_invalidate_all(
            [&] (const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(this->acq_sig_, blk_id);
            }
        );
        #endif
        
        MEFDN_LOG_DEBUG("msg:Exiting DSM barrier.");
    }
    
private:
    void acquire_sig(const sig_buffer_type& sig)
    {
        const auto min_wr_ts = sig.get_min_wr_ts();
        
        // Increase the acquire timestamp.
        // Note that write notices are not merged to the acquire signature
        // because they all are applied to all the processes
        // and hence they don't need to be transferred from this process to another.
        this->acq_sig_.acquire_barrier_sig(sig);
        
        // Self-invalidate all of the old blocks.
        this->rd_set_.self_invalidate(
            min_wr_ts,
            [&] (const blk_id_type blk_id) {
                return this->seg_tbl_.self_invalidate(this->acq_sig_, blk_id);
            }
        );
        
        // Invalidate the blocks based on write notices.
        sig.for_all_wns(
            [&] (const wn_entry_type& wn) {
                // Invalidate this block based on the write notice.
                // If invalidated, the home process ID and the timestamp are recorded.
                this->seg_tbl_.acquire(wn);
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
    
private:
    seg_table_type  seg_tbl_;
    
    wr_set_type     wr_set_;
    
    rd_set_type     rd_set_;
    
    rel_sig_type    rel_sig_;
    
    acq_sig_type    acq_sig_;
};

} // namespace medsm2
} // namespace menps

