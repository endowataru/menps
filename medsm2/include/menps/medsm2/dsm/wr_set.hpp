
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/algorithm.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/assert.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class wr_set
{
    using blk_id_type = typename P::blk_id_type;
    using size_type = typename P::size_type;
    
    using wr_set_gen_type   = typename P::wr_set_gen_type;
    
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    using wn_entry_type = typename P::wn_entry_type;
    using wn_vector_type = typename P::wn_vector_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::template unique_lock<mutex_type>;
    using cv_type = typename ult_itf_type::condition_variable;
    // Note: This class requires a condition variable to synchronize the threads
    // which request a release fence.
    // Spinlocks cannot be applied to this class.
    
public:
    wr_set() = default;
    
    void finalize() {
        // do nothing
    }
    
    void add_writable(const blk_id_type blk_id)
    {
        const unique_lock_type lk{this->new_ids_mtx_};
        this->new_ids_.push_back(blk_id);
    }
    
    struct start_release_result {
        bool            needs_release;
        wn_vector_type  wn_vec;
    };
    
    template <typename ReleaseFunc, typename MakeWnVecFunc>
    start_release_result start_release_for_all_blocks(
        const rd_ts_state_type& rd_ts_st
    ,   ReleaseFunc             rel_func
    ,   MakeWnVecFunc&&         make_wn_vec_func
    ) {
        MEFDN_STATIC_ASSERT(mefdn::is_signed<wr_set_gen_type>::value);
        
        {
            unique_lock_type lk{this->rel_mtx_};
            
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
                
                return { false, {} };
            }
            
            // This thread was selected for releasing this generation.
            this->is_releasing_ = true;
        }
        
        std::vector<blk_id_type> new_ids;
        {
            const unique_lock_type lk{this->new_ids_mtx_};
            
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
        
        using release_result_type =
            fdn::decay_t<decltype(rel_func(rd_ts_st, fdn::declval<blk_id_type>()))>;
        
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
        ,   [&rel_results, &new_dirty_ids, &rd_ts_st, num_released, stride, &rel_func]
            (const size_type first) {
                const auto last = mefdn::min(first + stride, num_released);
                for (size_type i = first; i < last; ++i) {
                    CMPTH_P_PROF_SCOPE(P, release);
                    
                    // Call the callback release function.
                    // The returned values are stored in parallel.
                    rel_results[i] = rel_func(rd_ts_st, new_dirty_ids[i]);
                }
            }
        );
        
        auto wn_vec = fdn::forward<MakeWnVecFunc>(make_wn_vec_func)(
            released_first, released_last, begin(rel_results));
        
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
        
        return { true, fdn::move(wn_vec) };
    }
    
    void finish_release()
    {
        MEFDN_ASSERT(this->is_releasing_);
        
        {
            const unique_lock_type lk{this->rel_mtx_};
            
            this->is_releasing_ = false;
            ++this->gen_;
            
            // Notify the other waiting threads.
            this->rel_cv_.notify_all();
        }
    }
    
private:
    mutex_type new_ids_mtx_;
    std::vector<blk_id_type> new_ids_;
    
    mutex_type rel_mtx_;
    cv_type rel_cv_;
    std::vector<blk_id_type> dirty_ids_;
    wr_set_gen_type gen_ = 0;
    bool is_releasing_ = false;
};

} // namespace medsm2
} // namespace menps

