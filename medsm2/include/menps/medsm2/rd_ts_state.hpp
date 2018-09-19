
#pragma once

#include <menps/medsm2/common.hpp>
#include <algorithm>

namespace menps {
namespace medsm2 {

template <typename P>
class rd_ts_state
{
    using rd_set_type = typename P::rd_set_type;
    
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
public:
    explicit rd_ts_state(rd_set_type& rd_set, const wr_ts_type min_wr_ts)
        : rd_set_(rd_set)
        , min_wr_ts_(min_wr_ts)
    { }
    
    rd_ts_state(const rd_ts_state&) = default;
    rd_ts_state& operator = (const rd_ts_state&) = default;
    
    rd_set_type& get_rd_set() const noexcept {
        return this->rd_set_;
    }
    
    wr_ts_type make_new_wr_ts(const rd_ts_type old_rd_ts) const noexcept {
        return std::max(old_rd_ts + 1, this->min_wr_ts_);
    }
    
    rd_ts_type make_new_rd_ts(const wr_ts_type wr_ts, const rd_ts_type rd_ts) const noexcept
    {
        // TODO: Provide a good prediction for a lease value of each block.
        return std::max(std::max(wr_ts, this->min_wr_ts_) + P::constants_type::lease_ts, rd_ts);
    }
    
    wr_ts_type get_min_wr_ts() const noexcept {
        return this->min_wr_ts_;
    }
    
    bool is_valid_rd_ts(const rd_ts_type rd_ts) const noexcept {
        // If rd_ts >= min_wr_ts, the block is still valid.
        return !P::is_greater_rd_ts(this->min_wr_ts_, rd_ts);
    }
    
private:
    rd_set_type&    rd_set_;
    wr_ts_type      min_wr_ts_;
};

} // namespace medsm2
} // namespace menps

