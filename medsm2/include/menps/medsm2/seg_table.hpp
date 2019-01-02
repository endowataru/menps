
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/medsm2/prof.hpp>

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
    using blk_lock_tbl_type = typename blk_tbl_type::lock_table_type;
    using blk_data_tbl_type = typename blk_tbl_type::data_table_type;
    using blk_dir_tbl_type = typename blk_tbl_type::dir_table_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    
    using blk_tbl_ptr = mefdn::unique_ptr<blk_tbl_type>;
    
    using rd_set_type = typename P::rd_set_type;
    using wr_set_type = typename P::wr_set_type;
    
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    struct lock_info
    {
        unique_lock_type    lk;
        blk_id_type         blk_id;
        blk_pos_type        blk_pos;
        blk_lock_tbl_type&  lock_tbl;
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
        auto& dir_tbl = blk_tbl.get_dir_tbl();
        
        const auto blk_pos = blk_tbl.get_blk_pos_from_blk_id(blk_id); // TODO
        
        return {
            dir_tbl.get_local_lock(blk_pos)
        ,   blk_id
        ,   blk_pos
        ,   blk_tbl.get_lock_tbl()
        ,   blk_tbl.get_data_tbl()
        ,   dir_tbl
        };
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
    start_read_result start_read(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   const blk_id_type   blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        return this->start_read_locked(com, rd_set, info);
    }
    
    struct start_write_result
    {
        start_read_result                               read;
        typename blk_dir_tbl_type::start_write_result   write;
    };
    
    MEFDN_NODISCARD
    start_write_result start_write(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   wr_set_type&        wr_set
    ,   const blk_id_type   blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        // Before writing the block, read the block first.
        // TODO: This call is not necessary in the current SIGSEGV handler
        //       because it cannot detect whether the fault is read or write.
        const auto read_ret = this->start_read_locked(com, rd_set, info);
        
        const auto rd_ts_st = rd_set.get_ts_state();
        
        return { read_ret, this->start_write_locked(wr_set, rd_ts_st, info) };
    }
    
    using pin_result = start_write_result;
    
    MEFDN_NODISCARD
    pin_result pin(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   wr_set_type&        wr_set
    ,   const blk_id_type   blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        // Start reading on this block before writing.
        const auto read_ret = this->start_read_locked(com, rd_set, info);
        
        const auto rd_ts_st = rd_set.get_ts_state();
        
        // Start writing on this block before releasing.
        const auto write_ret = this->start_write_locked(wr_set, rd_ts_st, info);
        
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
    start_read_result start_read_locked(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   const lock_info&    info
    ) {
        // Check whether this block is invalid or not.
        // The home process may be examined by following the probable owners.
        const auto start_ret =
            info.dir_tbl.start_read(rd_set, info.blk_id, info.blk_pos, info.lk);
        
        if (start_ret.needs_read) {
            // This block is inaccessible (= invalidated).
            // The latest data must be read from the home node.
            
            if (start_ret.needs_latest_read) {
                // This block was invalidated based on timestamps.
                
                const auto p = prof::start();
                
                const auto rd_ts_st = rd_set.get_ts_state();
                
                // Read & merge the latest values inside the global critical section.
                const auto tx_ret MEFDN_MAYBE_UNUSED /*TODO*/ =
                    this->do_transaction(com, rd_ts_st, info);
                // TODO: It is strange that tx_ret is totally ignored in this method.
                
                prof::finish(prof_kind::tx_read, p);
                
                MEFDN_LOG_DEBUG(
                    "msg:Start reading latest block.\t"
                    "blk_pos:{}\t"
                    "home_proc:{}\t"
                    "rd_ts:{}\t"
                    "{}"
                ,   info.blk_pos
                ,   start_ret.home_proc
                ,   start_ret.rd_ts
                ,   this->show_transaction_result(tx_ret)
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
                    "rd_ts:{}\t"
                    "is_dirty:{}"
                ,   info.blk_pos
                ,   start_ret.home_proc
                ,   start_ret.rd_ts
                ,   start_ret.is_dirty
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
    start_write_locked(
        wr_set_type&            wr_set
    ,   const rd_ts_state_type& rd_ts_st
    ,   const lock_info&        info
    ) {
        const auto dir_ret =
            info.dir_tbl.start_write(wr_set, rd_ts_st, info.blk_id, info.blk_pos, info.lk);
        
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
    acquire(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const wn_entry_type&    wn
    ) {
        const auto info = this->get_local_lock(wn.blk_id);
        
        // If the write notice is newer than the current directory's information,
        // the block is recorded as invalidated.
        const auto ret =
            info.dir_tbl.acquire(info.blk_pos, info.lk,
                wn.home_proc, wn.rd_ts, wn.wr_ts);
        
        if (ret.needs_merge) {
            // Do a merge for the pinned block.
            const auto tx_ret MEFDN_MAYBE_UNUSED /*TODO*/ =
                this->do_transaction(com, rd_ts_st, info);
            
            // TODO: How to deal with the result of this transaction?
        }
        // Check whether the protection is necessary based on this write notice.
        else if (ret.needs_protect) {
            // Make the data inaccessible from all of the threads.
            info.data_tbl.invalidate(info.blk_pos, info.lk);
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
    
    typename blk_dir_tbl_type::self_invalidate_result
    self_invalidate(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const blk_id_type       blk_id
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        const auto ret =
            info.dir_tbl.self_invalidate(rd_ts_st, info.blk_pos, info.lk);
        
        if (ret.needs_merge) {
            // Do a merge for the pinned block.
            const auto tx_ret MEFDN_MAYBE_UNUSED /*TODO*/ =
                this->do_transaction(com, rd_ts_st, info);
            
            // TODO: How to deal with the result of this transaction?
        }
        else if (ret.needs_protect) {
            // Make the data inaccessible from all of the threads.
            info.data_tbl.invalidate(info.blk_pos, info.lk);
        }
        
        MEFDN_LOG_DEBUG(
            "msg:{}.\t"
            "blk_pos:{}\t"
            "wr_ts:{}\t"
            "rd_ts:{}\t"
            "min_wr_ts:{}"
        ,   (ret.needs_protect ? "Self-invalidated block." :
                (ret.is_ignored ? "Avoid self-invalidation." :
                    "Self-invalidate invalid block."))
        ,   info.blk_pos
        ,   ret.wr_ts
        ,   ret.rd_ts
        ,   rd_ts_st.get_min_wr_ts()
        );
        
        return ret;
    }
    
    struct release_result {
        bool release_completed;
        bool is_written;
        // Indicate that this block can be removed from the write set.
        bool is_still_writable;
        rd_ts_type new_rd_ts;
        wr_ts_type new_wr_ts;
    };
    
    release_result release(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const blk_id_type       blk_id
    ) {
        const auto p = prof::start();
        
        const auto info = this->get_local_lock(blk_id);
        
        const auto check_ret =
            info.dir_tbl.check_release(info.blk_pos, info.lk);
        
        if (!check_ret.needs_release) {
            // This block is not released now.
            return { false, false, false, 0, 0 };
        }
        
        #ifdef MEDSM2_ENABLE_FAST_RELEASE
        // Check whether the data is still owned by this process.
        if (check_ret.is_fast_released
            && info.lock_tbl.check_owned(com, info.blk_pos, info.lk))
        {
            // Load and update the local timestamp values.
            // The updated values may be used in the next release.
            const auto fast_ret =
                info.dir_tbl.fast_release(rd_ts_st, info.blk_pos, info.lk);
            
            prof::finish(prof_kind::release_fast, p);
            
            return { true, true, true, fast_ret.new_rd_ts, fast_ret.new_wr_ts };
        }
        #endif
        
        // Merge the written values inside the global critical section.
        auto tx_ret =
            this->do_transaction(com, rd_ts_st, info);
        
        prof::finish(prof_kind::release_tx, p);
        
        MEFDN_LOG_DEBUG(
            "msg:Released block.\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "{}"
        ,   blk_id
        ,   info.blk_pos
        ,   this->show_transaction_result(tx_ret)
        );
        
        return {
            true
        ,   tx_ret.mg_ret.is_written
        ,   tx_ret.et_ret.is_still_writable
        ,   tx_ret.et_ret.new_rd_ts
        ,   tx_ret.et_ret.new_wr_ts
        };
    }
    
private:
    struct do_transaction_result {
        typename blk_lock_tbl_type::lock_global_result      glk_ret;
        typename blk_dir_tbl_type::begin_transaction_result bt_ret;
        typename blk_data_tbl_type::release_merge_result    mg_ret;
        typename blk_dir_tbl_type::end_transaction_result   et_ret;
    };
    
    template <typename Func>
    do_transaction_result do_transaction(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const lock_info&        info
    ,   Func&&                  func
    ) {
        const auto p_lock_global = prof::start();
        
        // Lock the latest owner globally.
        // This is achieved by following the graph of probable owners.
        const auto glk_ret =
            info.lock_tbl.lock_global(com, info.blk_id, info.blk_pos, info.lk);
        
        prof::finish(prof_kind::lock_global, p_lock_global);
        
        const auto p_begin_tx = prof::start();
       
        // Begin a transaction.
        const auto bt_ret =
            info.dir_tbl.begin_transaction(com, info.blk_pos, info.lk, glk_ret);
        
        prof::finish(prof_kind::begin_tx, p_begin_tx);
        
        const auto p_mg = prof::start();
        
        // Merge the writes from both the current process and the latest owner.
        /*const*/ auto mg_ret =
            info.data_tbl.release_merge(com, info.blk_pos, info.lk, bt_ret);
        
        // Invoke a special operation on this block.
        const auto is_changed =
            mefdn::forward<Func>(func)(info);
        
        if (is_changed) {
            // Set this flag if the atomic operation changed this block.
            mg_ret.is_written = true;
        }
        
        prof::finish(prof_kind::tx_merge, p_mg);
        
        const auto p_end_tx = prof::start();
        
        // Unlock the global lock.
        const auto et_ret =
            info.dir_tbl.end_transaction(com, rd_ts_st,
                info.blk_id, info.blk_pos, info.lk, bt_ret, mg_ret);
        
        prof::finish(prof_kind::end_tx, p_end_tx);
        
        const auto p_unlock_global = prof::start();
        
        // Unlock the global lock.
        info.lock_tbl.unlock_global(com, info.blk_id, info.blk_pos, info.lk, glk_ret, et_ret);
        
        prof::finish(prof_kind::unlock_global, p_unlock_global);
        
        return { glk_ret, bt_ret, mg_ret, et_ret };
    }
    
    struct do_transaction_default_functor
    {
        bool operator() (const lock_info& /*info*/) const noexcept {
            // Not changed.
            return false;
        }
    };
    
    do_transaction_result do_transaction(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const lock_info&        info
    ) {
        return do_transaction(com, rd_ts_st, info, do_transaction_default_functor{});
    }
    
    template <typename T, typename Func>
    do_transaction_result do_amo_at(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   const lock_info&        info
    ,   const size_type         offset
    ,   Func&&                  func
    ) {
        return this->do_transaction(com, rd_ts_st, info,
            [&func, offset] (const lock_info& info2) {
                return info2.data_tbl.template do_amo_at<T>(
                    info2.blk_pos
                ,   info2.lk
                ,   offset
                ,   mefdn::forward<Func>(func)
                );
            });
    }
    
    static std::string show_transaction_result(const do_transaction_result& tx_ret)
    {
        return fmt::format(
            "old_wr_ts:{}\t"
            "old_rd_ts:{}\t"
            "new_wr_ts:{}\t"
            "new_rd_ts:{}\t"
            "is_written:{}\t"
            "is_still_writable:{}\t"
        ,   tx_ret.bt_ret.wr_ts
        ,   tx_ret.bt_ret.rd_ts
        ,   tx_ret.et_ret.new_wr_ts
        ,   tx_ret.et_ret.new_rd_ts
        ,   tx_ret.mg_ret.is_written
        ,   tx_ret.et_ret.is_still_writable
        );
    }
    
public:
    struct compare_exchange_result
    {
        // Indicate that the CAS operation succeeded.
        bool is_success;
    };
    
    template <typename T>
    compare_exchange_result compare_exchange(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   const blk_id_type   blk_id
    ,   const size_type     offset
    ,   T&                  expected
    ,   const T             desired
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        bool is_success = false;
        const auto expected_val MEFDN_MAYBE_UNUSED = expected;
        
        const auto rd_ts_st = rd_set.get_ts_state();
        
        const auto tx_ret MEFDN_MAYBE_UNUSED /*TODO*/ =
            this->template do_amo_at<T>(com, rd_ts_st, info, offset,
                [&expected, desired, &is_success] (const T target) {
                    if (target == expected) {
                        is_success = true;
                        
                        // Replace the value of the atomic variable with "desired".
                        return desired;
                    }
                    else {
                        expected = target;
                        return target;
                    }
                });
        
        MEFDN_LOG_DEBUG(
            "msg:{}\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "offset:0x{}\t"
            "expected:{}\t"
            "desired:{}\t"
            "result:{}\t"
            "{}"
        ,   (is_success ? "DSM's CAS succeeded." : "DSM's CAS failed.")
        ,   blk_id
        ,   info.blk_pos
        ,   offset
        ,   expected_val
        ,   desired
        ,   expected
        ,   this->show_transaction_result(tx_ret)
        );
        
        return { is_success };
    }
    
    template <typename T>
    void atomic_store(
        com_itf_type&       com
    ,   rd_set_type&        rd_set
    ,   const blk_id_type   blk_id
    ,   const size_type     offset
    ,   const T             value
    ) {
        const auto info = this->get_local_lock(blk_id);
        
        const auto rd_ts_st = rd_set.get_ts_state();
        
        const auto tx_ret MEFDN_MAYBE_UNUSED /*TODO*/ =
            this->template do_amo_at<T>(com, rd_ts_st, info, offset,
                [value] (const T /*target*/) {
                    return value;
                });
        
        MEFDN_LOG_DEBUG(
            "msg:DSM's atomic store.\t"
            "blk_id:0x{:x}\t"
            "blk_pos:{}\t"
            "offset:0x{}\t"
            "value:{}\t"
            "{}"
        ,   blk_id
        ,   info.blk_pos
        ,   offset
        ,   value
        ,   this->show_transaction_result(tx_ret)
        );
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
    size_type get_blk_pos(const seg_id_type seg_id, const blk_id_type blk_id)
    {
        MEFDN_ASSERT(seg_id < this->blk_tbls_.size());
        MEFDN_ASSERT(this->blk_tbls_[seg_id]);
        return this->blk_tbls_[seg_id]->get_blk_pos_from_blk_id(blk_id);
    }
    
    #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    unique_lock_type get_flags_lock() {
        return unique_lock_type(this->flags_mtx_);
    }
    
    bool try_set_flag(const blk_id_type blk_id) {
        auto& self = this->derived();
        
        const auto seg_id = self.get_seg_id(blk_id);
        
        MEFDN_ASSERT(this->blk_tbls_[seg_id]);
        
        auto& blk_tbl = * this->blk_tbls_[seg_id];
        auto& flag_tbl = blk_tbl.get_flag_tbl();
        
        const auto blk_pos = blk_tbl.get_blk_pos_from_blk_id(blk_id); // TODO
        
        return flag_tbl.try_set(blk_pos);
    }
    
    void unset_flag(const blk_id_type blk_id) {
        auto& self = this->derived();
        
        const auto seg_id = self.get_seg_id(blk_id);
        
        MEFDN_ASSERT(this->blk_tbls_[seg_id]);
        
        auto& blk_tbl = * this->blk_tbls_[seg_id];
        auto& flag_tbl = blk_tbl.get_flag_tbl();
        
        const auto blk_pos = blk_tbl.get_blk_pos_from_blk_id(blk_id); // TODO
        
        flag_tbl.unset(blk_pos);
    }
    #endif
    
private:
    mefdn::vector<blk_tbl_ptr> blk_tbls_;
    
    #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    mutex_type flags_mtx_;
    #endif
};

} // namespace medsm2
} // namespace menps

