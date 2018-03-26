
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/vector.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class wr_set
{
    using blk_id_type = typename P::blk_id_type;
    
    using rel_pos_type   = typename P::rel_pos_type;
    
    using mutex_type        = typename P::mutex_type;
    using cv_type           = typename P::cv_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
public:
    wr_set() = default;
    
    void add_writable(const blk_id_type blk_id)
    {
        const unique_lock_type lk(this->mtx_);
        this->new_ids_.push_back(blk_id);
    }
    
    #if 0
    void add_ordered(const blk_id_type blk_id)
    {
        // TODO: Because this method is frequently called,
        //       it is better to avoid using a mutex (or a spinlock).
        const unique_lock_type lk(this->mtx_);
        
        // Record the new store-released block.
        // Note that the order of this push_back is also important for ensuring consistency.
        this->ordered_.push_back(blk_id);
        
        // Set this flag to wake up the releaser thread.
        this->need_rel_ = true;
        
        // Notify the releaser thread.
        this->start_rel_cv_.notify_one();
    }
    #endif
    
    void fence()
    {
        unique_lock_type lk(this->mtx_);
        
        // Record the current release position.
        const auto rel_pos = this->cur_rel_pos_;
        
        // Set this flag to wake up the releaser thread.
        this->need_rel_ = true;
        
        // Notify the releaser thread if it's sleeping.
        this->start_rel_cv_.notify_one();
        
        while (this->done_rel_pos_ == rel_pos) {
            // Wait for the completion of the release.
            this->fence_cv_.wait(lk);
        }
        
        // The release position changed.
    }
    
    struct release_result {
        bool                        finished;
        mefdn::vector<blk_id_type>  ordered_ids;
    };
    
    template <typename Func>
    release_result release_for_all_blocks(Func func)
    {
        mefdn::vector<blk_id_type> new_ids;
        mefdn::vector<blk_id_type> ordered;
        
        {
            unique_lock_type lk(this->mtx_);
            
            // Set the "done" release position to notify the fencing threads.
            this->done_rel_pos_ = this->cur_rel_pos_;
            
            // Notify all of the threads to see the latest release position.
            this->fence_cv_.notify_all();
            
            while (! this->need_rel_ && ! this->finished_) {
                // Wait for a new release request.
                this->start_rel_cv_.wait(lk);
            }
            
            if (this->finished_) {
                return { true, {} };
            }
            
            // Remove the content of new_ids_.
            new_ids = mefdn::move(this->new_ids_);
            
            // Remove the content of ordered IDs.
            ordered = mefdn::move(this->ordered_);
            
            // Generate the next release position.
            this->cur_rel_pos_++;
            
            // Reset this flag to accumulate the blocks for the next release.
            this->need_rel_ = false;
        }
        
        // Insert the block IDs to unordered_map.
        // Duplicated IDs are removed here.
        this->ids_.insert(mefdn::begin(new_ids), mefdn::end(new_ids));
        
        // TODO: This for-loop assumes that ids_ doesn't change
        //       because only one releaser thread exists.
        
        /*parallel*/ for (const auto& blk_id : this->ids_) {
            // Call the release function.
            func(blk_id);
        }
        
        return { false, ordered };
    }
    
    template <typename BlkIdItr>
    void remove(const BlkIdItr blk_id_first, const BlkIdItr blk_id_last)
    {
        for (auto blk_id_itr = blk_id_first; blk_id_itr != blk_id_last; ++blk_id_itr)
        {
            this->ids_.erase(*blk_id_itr);
        }
    }
    
    void finalize()
    {
        const unique_lock_type lk(this->mtx_);
        
        // Set this flag to destroy the releaser thread.
        this->finished_ = true;
        
        // Wake up the releaser thread if it's sleeping.
        this->start_rel_cv_.notify_one();
    }
    
private:
    std::unordered_set<blk_id_type> ids_;
        // TODO: Change the data structure if unnecessary
    
    mutex_type mtx_;
    cv_type start_rel_cv_;
    cv_type fence_cv_;
    // TODO: Use consistent names.
    mefdn::vector<blk_id_type> new_ids_;
    mefdn::vector<blk_id_type> ordered_;
    rel_pos_type cur_rel_pos_ = 0;
    rel_pos_type done_rel_pos_ = 0;
    bool need_rel_ = false;
    bool finished_ = false;
};

} // namespace medsm2
} // namespace menps

