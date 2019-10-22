
#pragma once

#include <menps/medsm2/common.hpp>
#include <queue>

namespace menps {
namespace medsm2 {

template <typename P>
class rd_set
{
    using rd_ts_state_type = typename P::rd_ts_state_type;
    
    using blk_id_type = typename P::blk_id_type;
    using rd_ts_type = typename P::rd_ts_type;
    using atomic_wr_ts_type = typename P::atomic_wr_ts_type;
    
    using size_type = typename P::size_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    using spinlock_guard_type = typename ult_itf_type::template lock_guard<spinlock_type>;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_guard_type = typename ult_itf_type::template lock_guard<mutex_type>;
    
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
        std::priority_queue<entry, std::vector<entry>, greater_ts>;
    
public:
    rd_set()
        : min_wr_ts_()
    { }
    
    void add_readable(const blk_id_type blk_id, const rd_ts_type rd_ts)
    {
        const mutex_guard_type lk{this->mtx_};
        
        // Check that (min_wr_ts <= rd_ts).
        MEFDN_ASSERT(!P::is_greater_rd_ts(this->min_wr_ts_.load(), rd_ts));
        
        // Add a new entry to the priority queue.
        this->pq_.push(entry{ blk_id, rd_ts });
    }
    
    template <typename Func>
    void self_invalidate(const rd_ts_type min_wr_ts, Func&& func)
    {
        std::vector<blk_id_type> blk_ids;
        
        {
            const mutex_guard_type lk{this->mtx_};
            
            const auto old_min_wr_ts =
                this->min_wr_ts_.load(ult_itf_type::memory_order_relaxed);
            
            if (!P::is_greater_rd_ts(min_wr_ts, old_min_wr_ts)) {
                // Because min_wr_ts <= old_min_wr_ts,
                // it is unnecessary to self-invalidate blocks for this signature.
                MEFDN_LOG_VERBOSE(
                    "msg:No need to self-invalidate.\t"
                    "old_min_wr_ts:{}\t"
                    "new_min_wr_ts:{}\t"
                ,   old_min_wr_ts
                ,   min_wr_ts
                );
                return;
            }
            
            // Update the minimum write timestamp.
            this->min_wr_ts_.store(min_wr_ts, ult_itf_type::memory_order_relaxed);
            
            while (! this->pq_.empty()) {
                auto& e = this->pq_.top();
                if (!P::is_greater_rd_ts(min_wr_ts, e.rd_ts)) {
                    // e.rd_ts is still valid.
                    break;
                }
                
                // Add the block ID if the block is a candidate for self-invalidation.
                blk_ids.push_back(e.blk_id);
                
                this->pq_.pop();
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Start self-invalidating all old blocks.\t"
            "min_wr_ts:{}\t"
            "num_blk_ids:{}"
        ,   min_wr_ts
        ,   blk_ids.size()
        );
        
        std::vector<entry> new_ents;
        spinlock_type new_ents_lock;
        
        const rd_ts_state_type rd_ts_st(*this, min_wr_ts);
        
        //for (const auto& blk_id : blk_ids) {
        ult_itf_type::for_loop(
            ult_itf_type::execution::seq
            //ult_itf_type::execution::par
            // TODO: This can be parallelized, but tasks are fine-grained
        ,   0
        ,   blk_ids.size()
        ,   [&func, &blk_ids, &new_ents, &new_ents_lock, min_wr_ts, &rd_ts_st] (const size_type i) {
                const auto blk_id = blk_ids[i];
                const auto ret = func(rd_ts_st, blk_id);
                if (!P::is_greater_rd_ts(min_wr_ts, ret.rd_ts)) {
                    spinlock_guard_type lk{new_ents_lock};
                    new_ents.push_back(entry{ blk_id, ret.rd_ts });
                }
            }
        );
        
        if (!new_ents.empty()) {
            const mutex_guard_type lk{this->mtx_};
            for (const auto& e : new_ents) {
                this->pq_.push(e);
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Finish self-invalidating all old blocks.\t"
            "min_wr_ts:{}\t"
            "num_old_blks:{}\t"
            "num_valid_blks:{}\t"
        ,   min_wr_ts
        ,   blk_ids.size()
        ,   new_ents.size()
        );
    }
    
    template <typename Func>
    void self_invalidate_all(Func&& func)
    {
        std::vector<blk_id_type> blk_ids;
        
        {
            const mutex_guard_type lk{this->mtx_};
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
    
    rd_ts_state_type get_ts_state()
    {
        // The minimum timestamp might be loaded concurrently with another writer.
        // To avoid a race condition, this class uses an atomic variable.
        const auto min_wr_ts =
            this->min_wr_ts_.load(ult_itf_type::memory_order_relaxed);
        
        return rd_ts_state_type(*this, min_wr_ts);
    }
    
private:
    mutable mutex_type  mtx_;
    pq_type             pq_;
    atomic_wr_ts_type   min_wr_ts_;
};

} // namespace medsm2
} // namespace menps

