
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/utility.hpp>
#include <stdexcept>

namespace menps {
namespace medsm2 {

template <typename P>
class seg_table
{
    MEFDN_DEFINE_DERIVED(P)
    
    using com_itf_type  = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using size_type = typename P::size_type;
    
    using seg_id_type = typename P::seg_id_type;
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
    using blk_tbl_type = typename P::blk_tbl_type;
    using blk_data_tbl_type = typename blk_tbl_type::data_table_type;
    using blk_dir_tbl_type = typename blk_tbl_type::dir_table_type;
    
    using unique_lock_type = typename P::unique_lock_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    
    using blk_tbl_ptr = mefdn::unique_ptr<blk_tbl_type>;
    
    using acq_sig_type = typename P::acq_sig_type;
    
    struct lock_info
    {
        unique_lock_type    lk;
        blk_pos_type        blk_pos;
        blk_data_tbl_type&  data_tbl;
        blk_dir_tbl_type&   dir_tbl;
        
        lock_info() = default;
        
        lock_info(lock_info&&) = default;
        
        ~lock_info() {
            if (lk.owns_lock()) {
                MEFDN_LOG_VERBOSE(
                    "msg:Unlocking dir lock.\t"
                    "blk_pos:{}"
                ,   blk_pos
                );
            }
        }
    };
    
    lock_info get_local_lock(const blk_id_type blk_id)
    {
        auto& self = this->derived();
        
        const auto seg_id = self.get_seg_id(blk_id);
        
        MEFDN_ASSERT(this->blk_tbls_[seg_id]);
        
        auto& blk_tbl = * this->blk_tbls_[seg_id];
        auto& data_tbl = blk_tbl.get_data_tbl();
        auto& dir_tbl = blk_tbl.get_dir_tbl();
        
        const auto blk_pos = blk_tbl.get_blk_pos_from_blk_id(blk_id); // TODO
        
        lock_info info{ dir_tbl.get_local_lock(blk_pos), blk_pos, data_tbl, dir_tbl };
        return info;
    }
    
public:
    template <typename Conf>
    explicit seg_table(const Conf& /*conf*/)
        //: blk_tbls_(conf.)
        : blk_tbls_(
            /*max_num_segs == */
            P::constants_type::max_space_size / P::constants_type::max_seg_size
        ) // TODO: Do not calculate this twice
    { }
    
    void finalize(com_itf_type& com)
    {
        for (auto& blk_tbl : this->blk_tbls_) {
            if (blk_tbl) {
                blk_tbl->finalize(com);
            }
        }
    }
    
    struct start_read_result {
        // Indicate that this block was invalid and now becomes readable.
        // This flag is used for upgrading the block in a segmentation fault.
        bool is_newly_read;
        // The read timestamp for this block.
        rd_ts_type rd_ts;
    };
    
    MEFDN_NODISCARD
    start_read_result start_read(com_itf_type& com, const acq_sig_type& acq_sig, const blk_id_type blk_id)
    {
        const auto info = this->get_local_lock(blk_id);
        
        return this->start_read_locked(com, acq_sig, info);
    }
    
    struct start_write_result
    {
        start_read_result                               read;
        typename blk_dir_tbl_type::start_write_result   write;
    };
    
    MEFDN_NODISCARD
    start_write_result start_write(com_itf_type& com, const acq_sig_type& acq_sig, const blk_id_type blk_id)
    {
        const auto info = this->get_local_lock(blk_id);
        
        // Before writing the block, read the block first.
        // TODO: This call is not necessary in the current SIGSEGV handler
        //       because it cannot detect whether the fault is read or write.
        const auto read_ret = this->start_read_locked(com, acq_sig, info);
        
        return { read_ret, this->start_write_locked(info) };
    }
    
    #if 0
    template <typename StoreOp>
    MEFDN_NODISCARD
    bool try_store_release(com_itf_type& com, const acq_sig_type& acq_sig, const blk_id_type blk_id, StoreOp&& store_op)
    {
        const auto info = this->get_local_lock(blk_id);
        
        // Start reading on this block before writing.
        this->start_read_locked(com, acq_sig, info);
        
        // Start writing on this block before releasing.
        this->start_write_locked(info);
        
        // Mark this block to be released at last.
        // If the block is already released, this function will return "false".
        const bool succeeded =
            info.dir_tbl.try_set_released(info.blk_pos, info.lk);
        
        // Invoke the store operation here before unlocking the block.
        mefdn::forward<StoreOp>(store_op)();
        
        return succeeded;
    }
    #endif
    
    using pin_result = start_write_result;
    
    MEFDN_NODISCARD
    pin_result pin(com_itf_type& com, const acq_sig_type& acq_sig, const blk_id_type blk_id)
    {
        const auto info = this->get_local_lock(blk_id);
        
        // Start reading on this block before writing.
        const auto read_ret = this->start_read_locked(com, acq_sig, info);
        
        // Start writing on this block before releasing.
        const auto write_ret = this->start_write_locked(info);
        
        // Mark this block as "pinned".
        info.dir_tbl.set_pinned(info.blk_pos, info.lk);
        
        MEFDN_LOG_DEBUG(
            "msg:Pinned block.\t"
            "blk_id:0x{:x}"
        ,   blk_id
        );
        
        return { read_ret, write_ret };
    }
    
    void unpin(const blk_id_type blk_id)
    {
        const auto info = this->get_local_lock(blk_id);
        
        // Mark this block as "unpinned".
        info.dir_tbl.set_unpinned(info.blk_pos, info.lk);
        
        MEFDN_LOG_DEBUG(
            "msg:Unpinned block.\t"
            "blk_id:0x{:x}"
        ,   blk_id
        );
    }
    
private:
    MEFDN_NODISCARD
    start_read_result start_read_locked(com_itf_type& com, const acq_sig_type& acq_sig, const lock_info& info)
    {
        // Check whether this block is invalid or not.
        // The home process may be examined by following the probable owners.
        const auto start_ret =
            info.dir_tbl.start_read(com, acq_sig, info.blk_pos, info.lk);
        
        if (start_ret.needs_read) {
            // This block is inaccessible (= invalidated).
            // The latest data must be read from the home node.
            
            if (start_ret.needs_latest_read) {
                // This block was invalidated based on timestamps.
                
                // Read & merge the latest values inside the global critical section.
                const auto tx_ret =
                    this->do_transaction(com, acq_sig, info,
                        false /* needs_protect_before is always false for invalid blocks */
                        // TODO: This flag complicates the logic.
                    );
                // TODO: It is strange that tx_ret is totally ignored in this method.
                
                // TODO: Do refactoring and remove this function.
                info.data_tbl.set_readonly(info.blk_pos, info.lk);
                
                MEFDN_LOG_DEBUG(
                    "msg:Start reading latest block.\t"
                    "blk_pos:{}\t"
                    "home_proc:{}\t"
                    "rd_ts:{}"
                ,   info.blk_pos
                ,   start_ret.home_proc
                ,   start_ret.rd_ts
                );
            }
            else {
                // Although this block was invalidated,
                // its read timestamp for this process is still alive.
                
                // If this process is not the home process, read from the home first.
                // After that, the private area of this block becomes read-only.
                info.data_tbl.start_read(com, info.blk_pos, info.lk,
                    start_ret.home_proc, start_ret.is_dirty);
                
                MEFDN_LOG_DEBUG(
                    "msg:Start reading block.\t"
                    "blk_pos:{}\t"
                    "home_proc:{}\t"
                    "rd_ts:{}"
                ,   info.blk_pos
                ,   start_ret.home_proc
                ,   start_ret.rd_ts
                );
            }
        }
        
        return {
            start_ret.needs_read
        ,   start_ret.rd_ts
            // FIXME: rd_ts must be updated. Return the new value here.
        };
    }
    
    MEFDN_NODISCARD
    typename blk_dir_tbl_type::start_write_result
    start_write_locked(const lock_info& info)
    {
        const auto dir_ret = info.dir_tbl.start_write(info.blk_pos, info.lk);
        
        // Check whether this block is read-only.
        if (dir_ret.needs_protect) {
            // This block must be upgraded.
            
            // Copy the data of the private area to the public area
            // and then make the private area writable.
            info.data_tbl.start_write(info.blk_pos, info.lk, dir_ret.needs_twin);
            
            MEFDN_LOG_DEBUG(
                "msg:Start writing block.\t"
                "blk_pos:{}"
            ,   info.blk_pos
            );
        }
        
        return dir_ret;
    }
    
public:
    typename blk_dir_tbl_type::acquire_result
    acquire(const wn_entry_type& wn)
    {
        const auto info = this->get_local_lock(wn.blk_id);
        
        // If the write notice is newer than the current directory's information,
        // the block is recorded as invalidated.
        const auto ret =
            info.dir_tbl.acquire(info.blk_pos, info.lk,
                wn.home_proc, wn.rd_ts, wn.wr_ts);
        
        // Check whether the protection is necessary based on this write notice.
        if (ret.needs_protect) {
            // Make the data inaccessible from all of the threads.
            info.data_tbl.invalidate(info.blk_pos, info.lk);
        }
        if (ret.needs_merge) {
            // FIXME
            throw std::logic_error("unimplemented");
        }
        
        if (! ret.is_ignored) {
            MEFDN_LOG_DEBUG(
                "msg:Invalidated block using WN.\t"
                "blk_pos:{}"
                "home_proc:{}\t"
                "wn_rd_ts:{}\t"
                "wn_wr_ts:{}"
            ,   info.blk_pos
            ,   wn.home_proc
            ,   wn.rd_ts
            ,   wn.wr_ts
            );
        }
        else {
            MEFDN_LOG_DEBUG(
                "msg:Ignored WN.\t"
                "blk_pos:{}\t"
                "home_proc:{}\t"
                "wn_rd_ts:{}\t"
                "wn_wr_ts:{}"
            ,   info.blk_pos
            ,   wn.home_proc
            ,   wn.rd_ts
            ,   wn.wr_ts
            );
        }
        
        return ret;
    }
    
    struct release_result {
        bool release_completed;
        bool is_written;
        // Indicate that this block can be removed from the write set.
        bool is_clean;
        rd_ts_type new_rd_ts;
        wr_ts_type new_wr_ts;
    };
    
    release_result release(
        com_itf_type&           com
    ,   const acq_sig_type&     acq_sig
    ,   const blk_id_type       blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        const auto check_ret =
            info.dir_tbl.check_release(info.blk_pos, info.lk);
        
        if (!check_ret.needs_release) {
            // This block is not released now.
            return { false, false, check_ret.is_clean, 0, 0 };
        }
        
        // Merge the written values inside the global critical section.
        auto tx_ret =
            this->do_transaction(com, acq_sig, info, check_ret.needs_protect_before);
        
        MEFDN_LOG_DEBUG(
            "msg:Released block.\t"
            "blk_id:0x{:x}\t"
            "old_wr_ts:{}\t"
            "old_rd_ts:{}\t"
            "new_wr_ts:{}\t"
            "new_rd_ts:{}\t"
            "is_migrated:{}\t"
            "is_written:{}\t"
            "is_clean:{}\t"
            "becomes_clean:{}"
        ,   blk_id
        ,   tx_ret.glk_ret.wr_ts
        ,   tx_ret.glk_ret.rd_ts
        ,   tx_ret.gunlk_ret.new_wr_ts
        ,   tx_ret.gunlk_ret.new_rd_ts
        ,   tx_ret.mg_ret.is_migrated
        ,   tx_ret.mg_ret.is_written
        ,   check_ret.is_clean
        ,   tx_ret.mg_ret.becomes_clean
        );
        
        return {
            true
        ,   tx_ret.mg_ret.is_written
        ,   tx_ret.mg_ret.becomes_clean
        ,   tx_ret.gunlk_ret.new_rd_ts
        ,   tx_ret.gunlk_ret.new_wr_ts
        };
    }
    
private:
    struct do_transaction_result {
        typename blk_dir_tbl_type::lock_global_result   glk_ret;
        typename blk_data_tbl_type::merge_to_result     mg_ret;
        typename blk_dir_tbl_type::unlock_global_result gunlk_ret;
    };
    
    do_transaction_result do_transaction(
        com_itf_type&       com
    ,   const acq_sig_type& acq_sig
    ,   const lock_info&    info
    ,   const bool          needs_protect_before
    ) {
        // Lock the latest owner globally.
        // This is achieved by following the graph of probable owners.
        const auto glk_ret =
            info.dir_tbl.lock_global(com, info.blk_pos, info.lk);
        
        // Merge the writes from both the current process and the latest owner.
        const auto mg_ret =
            info.data_tbl.merge_to_public(com, info.blk_pos, info.lk, glk_ret.owner,
                needs_protect_before);
        
        // Unlock the global lock.
        const auto gunlk_ret =
            info.dir_tbl.unlock_global(com, acq_sig, info.blk_pos, info.lk, glk_ret, mg_ret);
        
        return { glk_ret, mg_ret, gunlk_ret };
    }
    
public:
    typename blk_dir_tbl_type::self_invalidate_result
    self_invalidate(
        const acq_sig_type&     acq_sig
    ,   const blk_id_type       blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        const auto ret =
            info.dir_tbl.self_invalidate(acq_sig, info.blk_pos, info.lk);
        
        if (ret.needs_protect) {
            // Make the data inaccessible from all of the threads.
            info.data_tbl.invalidate(info.blk_pos, info.lk);
            
            MEFDN_LOG_DEBUG(
                "msg:Self-invalidated block.\t"
                "blk_pos:{}\t"
                "wr_ts:{}\t"
                "rd_ts:{}\t"
                "min_wr_ts:{}"
            ,   info.blk_pos
            ,   ret.wr_ts
            ,   ret.rd_ts
            ,   acq_sig.get_min_wr_ts()
            );
        }
        else {
            MEFDN_LOG_DEBUG(
                "msg:{}.\t"
                "blk_pos:{}\t"
                "wr_ts:{}\t"
                "rd_ts:{}\t"
                "min_wr_ts:{}"
            ,   (ret.is_ignored ? "Avoid self-invalidation" : "Self-invalidate invalid block")
            ,   info.blk_pos
            ,   ret.wr_ts
            ,   ret.rd_ts
            ,   acq_sig.get_min_wr_ts()
            );
        }
        
        if (ret.needs_merge) {
            // FIXME
            throw std::logic_error("unimplemented");
        }
        
        return ret;
    }
    
    void set_blk_table(const seg_id_type seg_id, blk_tbl_ptr blk_tbl)
    {
        MEFDN_ASSERT(seg_id < this->blk_tbls_.size());
        this->blk_tbls_[seg_id] = mefdn::move(blk_tbl);
    }
    size_type get_blk_size(const seg_id_type seg_id)
    {
        MEFDN_ASSERT(seg_id < this->blk_tbls_.size());
        MEFDN_ASSERT(this->blk_tbls_[seg_id]);
        return this->blk_tbls_[seg_id]->get_blk_size();
    }
    
private:
    mefdn::vector<blk_tbl_ptr> blk_tbls_;
};

} // namespace medsm2
} // namespace menps

