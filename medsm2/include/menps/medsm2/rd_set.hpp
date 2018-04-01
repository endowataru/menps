
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/vector.hpp>
#include <queue>

namespace menps {
namespace medsm2 {

template <typename P>
class rd_set
{
    using blk_id_type = typename P::blk_id_type;
    using rd_ts_type = typename P::rd_ts_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
    struct entry {
        blk_id_type blk_id;
        rd_ts_type  rd_ts;
    };
    
    struct greater_ts {
        bool operator() (const entry& a, const entry& b) const noexcept {
            return P::is_greater_rd_ts(a.rd_ts, b.rd_ts);
        }
    };
    
    using pq_type =
        std::priority_queue<entry, mefdn::vector<entry>, greater_ts>;
    
public:
    void add_readable(const blk_id_type blk_id, const rd_ts_type rd_ts)
    {
        const unique_lock_type lk(this->mtx_);
        
        // Add a new entry to the priority queue.
        this->pq_.push(entry{ blk_id, rd_ts });
    }
    
    template <typename Func>
    void self_invalidate(const rd_ts_type min_rd_ts, Func&& func)
    {
        MEFDN_LOG_DEBUG(
            "msg:Start self-invalidating all old blocks.\t"
            "min_rd_ts:{}\t"
        ,   min_rd_ts
        );
        
        mefdn::vector<blk_id_type> blk_ids;
        
        {
            const unique_lock_type lk(this->mtx_);
            while (! this->pq_.empty()) {
                auto& e = this->pq_.top();
                if (!P::is_greater_rd_ts(min_rd_ts, e.rd_ts)) {
                    // e.rd_ts is still valid.
                    break;
                }
                
                // Add the block ID if the block is a candidate for self-invalidation.
                blk_ids.push_back(e.blk_id);
                
                this->pq_.pop();
            }
        }
        
        mefdn::vector<entry> new_ents;
        
        for (const auto& blk_id : blk_ids) {
            const auto ret = func(blk_id);
            if (!P::is_greater_rd_ts(min_rd_ts, ret.rd_ts)) {
                new_ents.push_back(entry{ blk_id, ret.rd_ts });
            }
        }
        
        if (!new_ents.empty()) {
            const unique_lock_type lk(this->mtx_);
            for (const auto& e : new_ents) {
                this->pq_.push(e);
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Finish self-invalidating all old blocks.\t"
            "min_rd_ts:{}\t"
            "num_old_blks:{}\t"
            "num_valid_blks:{}\t"
        ,   min_rd_ts
        ,   blk_ids.size()
        ,   new_ents.size()
        );
    }
    
    template <typename Func>
    void self_invalidate_all(Func&& func)
    {
        mefdn::vector<blk_id_type> blk_ids;
        
        {
            const unique_lock_type lk(this->mtx_);
            while (!this->pq_.empty()) {
                const auto& e = this->pq_.top();
                blk_ids.push_back(e.blk_id);
                this->pq_.pop();
            }
        }
        
        for (const auto& blk_id : blk_ids) {
            // TODO: Ignoring the returned value.
            func(blk_id);
        }
    }
    
private:
    mutex_type  mtx_;
    pq_type     pq_;
};

} // namespace medsm2
} // namespace menps

