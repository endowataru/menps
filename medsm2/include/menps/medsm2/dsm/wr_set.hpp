
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/algorithm.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class wr_set
{
    using blk_id_type = typename P::blk_id_type;
    using size_type = typename P::size_type;
    
    using wr_set_gen_type   = typename P::wr_set_gen_type;
    
    using com_itf_type = typename P::com_itf_type;
    
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using ult_itf_type      = typename P::ult_itf_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    // This class requires a condition variable to synchronize the threads
    // which request a release fence.
    // Spinlocks cannot be applied to this class.
    using ult_mutex_type        = typename ult_itf_type::mutex;
    using ult_cv_type           = typename ult_itf_type::condition_variable;
    using ult_unique_lock_type  = typename ult_itf_type::unique_mutex_lock;
    
public:
    wr_set() = default;
    
    void finalize() {
        // do nothing
    }
    
    void add_writable(const blk_id_type blk_id)
    {
        const unique_lock_type lk(this->new_ids_mtx_);
        this->new_ids_.push_back(blk_id);
    }
    
    struct start_release_result {
        bool needs_release;
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        wn_vector_type wn_vec;
        #endif
    };
    
    template <typename SegTable>
    start_release_result start_release_for_all_blocks(
        com_itf_type&           com
    ,   const rd_ts_state_type& rd_ts_st
    ,   SegTable&               seg_table
    ) {
        MEFDN_STATIC_ASSERT(mefdn::is_signed<wr_set_gen_type>::value);
        
        {
            ult_unique_lock_type lk(this->rel_mtx_);
            
            // Decide the release position.
            const auto gen = this->gen_ + (this->is_releasing_ ? 1 : 0);
            
            // Wait until the previous release is completed.
            while (this->gen_ - gen < 0) {
                this->rel_cv_.wait(lk);
            }
            // Before this loop exits,
            // all of the preceding release operations must be completed.
            
            if (this->is_releasing_) {
                // This thread was not selected for releasing in this generation.
                
                // Wait until the selected thread completes this generation.
                while (this->gen_ - gen <= 0) {
                    this->rel_cv_.wait(lk);
                }
                
                return { false
                    #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
                    , {}
                    #endif
                    };
            }
            
            // This thread was selected for releasing this generation.
            this->is_releasing_ = true;
        }
        
        std::vector<blk_id_type> new_ids;
        {
            const unique_lock_type lk(this->new_ids_mtx_);
            
            // Remove all of the entries queued as new writable block IDs.
            new_ids = mefdn::move(this->new_ids_);
        }
        
        using mefdn::begin;
        using mefdn::end;
        
        // Sort the newly added IDs to prepare for the following merge.
        std::sort(begin(new_ids), end(new_ids));
        
        // Preallocate a sufficient space to hold new dirty IDs.
        std::vector<blk_id_type> new_dirty_ids;
        new_dirty_ids.reserve(this->dirty_ids_.size() + new_ids.size());
        
        // Merge the two sorted arrays for dirty IDs.
        std::merge(
            begin(this->dirty_ids_), end(this->dirty_ids_),
            begin(new_ids), end(new_ids),
            std::back_inserter(new_dirty_ids)
        );
        
        const auto released_first = begin(new_dirty_ids);
        
        // Remove the duplicates from the new ID array.
        // Note that the container itself still holds the duplicated elements.
        const auto released_last =
            std::unique(released_first, end(new_dirty_ids));
        // TODO: This can be simultaneously processed in the previous merge.
        
        const auto num_released =
            static_cast<size_type>(
                released_last - released_first
            );
        
        using release_result_type = typename SegTable::release_result;
        
        // Allocate an array to hold the results of release operations.
        std::vector<release_result_type> rel_results(num_released);
        
        const auto num_workers = ult_itf_type::get_num_workers();
        auto stride = num_released / (16 * num_workers); // TODO: magic number
        if (stride == 0) { stride = 1; }
        
        // Do the release operations in parallel.
        ult_itf_type::for_loop_strided(
            ult_itf_type::execution::par
        ,   0, num_released, stride
            // TODO: Too many captured variables...
        ,   [&rel_results, &new_dirty_ids, &com, &rd_ts_st, &seg_table, num_released, stride]
            (const size_type first) {
                const auto last = mefdn::min(first + stride, num_released);
                for (size_type i = first; i < last; ++i) {
                    const auto p = prof::start();
                    
                    // Call the callback release function.
                    // The returned values are stored in parallel.
                    rel_results[i] = seg_table.release(com, rd_ts_st, new_dirty_ids[i]);
                    
                    prof::finish(prof_kind::release, p);
                }
            }
        );
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        wn_vector_type wn_vec;
        // Pre-allocate the write notice vector.
        wn_vec.reserve(num_released);
        
        // Check all of the release results sequentially.
        for (size_type i = 0; i < num_released; ++i) {
            const auto blk_id = new_dirty_ids[i];
            
            const auto& rel_ret = rel_results[i];
            
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
        
        // Remove non-writable blocks from the dirty ID array.
        const auto dirty_last =
            std::remove_if(released_first, released_last,
                [&released_first, &rel_results] (const blk_id_type& blk_id) {
                    // Determine the index for the ID array.
                    const auto idx =
                        static_cast<size_type>(
                            &blk_id - &*released_first
                            // TODO: Simplify this pointer operation
                            //       using "induction" of for-loops.
                        );
                    
                    const auto& rel_ret = rel_results[idx];
                    
                    // Check the corresponding flag.
                    const auto is_dirty = rel_ret.is_still_writable;
                    
                    // Remove if the block is/becomes not dirty now.
                    // remove_if() removes an element if the returned value is true.
                    return ! is_dirty;
                });
        
        // Update the container to remove the.elements.
        new_dirty_ids.erase(dirty_last, end(new_dirty_ids));
        
        this->dirty_ids_ = mefdn::move(new_dirty_ids);
        
        return { true
            #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
            , wn_vec
            #endif
            };
    }
    
    void finish_release()
    {
        MEFDN_ASSERT(this->is_releasing_);
        
        {
            const ult_unique_lock_type lk(this->rel_mtx_);
            
            this->is_releasing_ = false;
            ++this->gen_;
            
            // Notify the other waiting threads.
            this->rel_cv_.notify_all();
        }
    }
    
private:
    mutex_type new_ids_mtx_;
    std::vector<blk_id_type> new_ids_;
    
    ult_mutex_type rel_mtx_;
    ult_cv_type rel_cv_;
    std::vector<blk_id_type> dirty_ids_;
    wr_set_gen_type gen_ = 0;
    bool is_releasing_ = false;
};

} // namespace medsm2
} // namespace menps

