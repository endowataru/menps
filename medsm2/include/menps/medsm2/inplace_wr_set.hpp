
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/vector.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class inplace_wr_set
{
    using blk_id_type = typename P::blk_id_type;
    
    using wr_set_gen_type   = typename P::wr_set_gen_type;
    
    using mutex_type        = typename P::mutex_type;
    using cv_type           = typename P::cv_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
public:
    inplace_wr_set() = default;
    
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
        
        // Insert the block IDs to unordered_map.
        // Duplicated IDs are removed here.
        this->dirty_ids_.insert(mefdn::begin(new_ids), mefdn::end(new_ids));
        
        // Do the release operations.
        /*parallel*/ for (const auto& blk_id : this->dirty_ids_) {
            // Call the release function.
            func(blk_id);
        }
        
        return { true };
    }
    
    template <typename BlkIdItr>
    void remove(const BlkIdItr blk_id_first, const BlkIdItr blk_id_last)
    {
        MEFDN_ASSERT(this->is_releasing_);
        
        for (auto blk_id_itr = blk_id_first; blk_id_itr != blk_id_last; ++blk_id_itr)
        {
            this->dirty_ids_.erase(*blk_id_itr);
        }
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
    std::unordered_set<blk_id_type> dirty_ids_;
    wr_set_gen_type gen_ = 0;
    bool is_releasing_ = false;
};

} // namespace medsm2
} // namespace menps

