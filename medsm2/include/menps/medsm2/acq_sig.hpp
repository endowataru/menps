
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class acq_sig
{
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
    using sig_buffer_type = typename P::sig_buffer_type;
    
    using mutex_type        = typename P::mutex_type;
    using mutex_unique_lock_type = typename P::mutex_unique_lock_type;
    
public:
    bool is_valid_rd_ts(rd_ts_type rd_ts) const
    {
        const mutex_unique_lock_type lk(this->mtx_);
        const auto min_wr_ts = this->sig_.get_min_wr_ts();
        
        return min_wr_ts < rd_ts;
    }
    
    wr_ts_type make_new_wr_ts(const rd_ts_type old_rd_ts) const
    {
        const mutex_unique_lock_type lk(this->mtx_);
        const auto min_wr_ts = this->sig_.get_min_wr_ts();
        
        return std::max(old_rd_ts + 1, min_wr_ts);
    }
    
    // TODO: This method doesn't depend on this class.
    rd_ts_type make_new_rd_ts(const wr_ts_type wr_ts, const rd_ts_type rd_ts) const
    {
        return std::max(wr_ts + P::constants_type::lease_ts, rd_ts);
    }
    
    wr_ts_type get_min_wr_ts() const noexcept {
        const mutex_unique_lock_type lk(this->mtx_);
        return this->sig_.get_min_wr_ts();
    }
    
    // TODO: atomics are unimplemented
    
private:
    mutable mutex_type  mtx_;
    sig_buffer_type     sig_;
};

} // namespace medsm2
} // namespace menps

