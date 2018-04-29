
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/vector.hpp>
#include <unordered_set>
#include <algorithm>

namespace menps {
namespace medsm2 {

template <typename P>
class wr_set
{
    using blk_id_type = typename P::blk_id_type;
    using size_type = typename P::size_type;
    
    using wr_set_gen_type   = typename P::wr_set_gen_type;
    
    using ult_itf_type      = typename P::ult_itf_type;
    
    using mutex_type        = typename P::mutex_type;
    using cv_type           = typename P::cv_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
    
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
    };
    
    template <typename Func>
    start_release_result start_release_for_all_blocks(Func func)
    {
        MEFDN_STATIC_ASSERT(mefdn::is_signed<wr_set_gen_type>::value);
        
        {
            unique_lock_type lk(this->rel_mtx_);
            
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
                
                return { false };
            }
            
            // This thread was selected for releasing this generation.
            this->is_releasing_ = true;
        }
        
        mefdn::vector<blk_id_type> new_ids;
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
        mefdn::vector<blk_id_type> new_dirty_ids;
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
        
        // Allocate an array to hold the dirty states.
        // std::vector<bool> should not be used here
        // because its elements cannot be assigned in parallel.
        const auto dirty_flags =
            mefdn::make_unique<bool []>(num_released);
        
        // Do the release operations in parallel.
        ult_itf_type::for_loop(
            ult_itf_type::execution::par
        ,   0, num_released
        ,   [&dirty_flags, &new_dirty_ids, &func] (const size_type i) {
                // Call the callback release function.
                // The returned values are stored in parallel.
                dirty_flags[i] = func(new_dirty_ids[i]);
            }
        );
        
        // Remove non-writable blocks from the dirty ID array.
        const auto dirty_last =
            std::remove_if(released_first, released_last,
                [&released_first, &dirty_flags] (const blk_id_type& blk_id) {
                    // Determine the index for the ID array.
                    const auto idx =
                        static_cast<size_type>(
                            &blk_id - &*released_first
                            // TODO: Simplify this pointer operation
                            //       using "induction" of for-loops.
                        );
                    
                    // Check the corresponding flag.
                    const auto is_dirty = dirty_flags[idx];
                    
                    // Remove if the block is/becomes not dirty now.
                    // remove_if() removes an element if the returned value is true.
                    return ! is_dirty;
                });
        
        // Update the container to remove the.elements.
        new_dirty_ids.erase(dirty_last, end(new_dirty_ids));
        
        this->dirty_ids_ = mefdn::move(new_dirty_ids);
        
        return { true };
    }
    
    void finish_release()
    {
        MEFDN_ASSERT(this->is_releasing_);
        
        {
            const unique_lock_type lk(this->rel_mtx_);
            
            this->is_releasing_ = false;
            ++this->gen_;
            
            // Notify the other waiting threads.
            this->rel_cv_.notify_all();
        }
    }
    
private:
    mutex_type new_ids_mtx_;
    mefdn::vector<blk_id_type> new_ids_;
    
    mutex_type rel_mtx_;
    cv_type rel_cv_;
    mefdn::vector<blk_id_type> dirty_ids_;
    wr_set_gen_type gen_ = 0;
    bool is_releasing_ = false;
};

} // namespace medsm2
} // namespace menps

