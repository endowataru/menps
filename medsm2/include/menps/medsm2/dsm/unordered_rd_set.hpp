
#pragma once

#include <menps/medsm2/common.hpp>
#include <unordered_set>

namespace menps {
namespace medsm2 {

template <typename P>
class unordered_rd_set
{
    using blk_id_type = typename P::blk_id_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
public:
    void add_readable(const blk_id_type blk_id) {
        const unique_lock_type lk(this->mtx_);
        this->blk_ids_.insert(blk_id);
    }

    template <typename Func>
    void self_invalidate(Func&& func)
    {
        const unique_lock_type lk(this->mtx_);
        const auto rd_ts_st = this->get_ts_state();
        std::vector<blk_id_type> removed;
        for (const auto blk_id : this->blk_ids_) {
            const auto ret = func(rd_ts_st, blk_id);
            if (!ret.is_ignored) {
                removed.push_back(blk_id);
            }
        }
        for (const auto blk_id : removed) {
            this->blk_ids_.erase(blk_id);
        }
    }

    struct rd_ts_state {
        unordered_rd_set& self;
        unordered_rd_set& get_rd_set() const noexcept {
            return self;
        }   
    };
    
    rd_ts_state get_ts_state() {
        return rd_ts_state{*this};
    }
    
private:
    mutable mutex_type              mtx_;
    std::unordered_set<blk_id_type> blk_ids_;
};

} // namespace medsm2
} // namespace menps
