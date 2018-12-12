
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class blk_flag_table
{
    using blk_pos_type = typename P::blk_pos_type;
    
    using size_type = typename P::size_type;
    
public:
    void init(const size_type num_blks)
    {
        this->flags_ = mefdn::make_unique<bool []>(num_blks);
    }
    
    bool try_set(const blk_pos_type blk_pos)
    {
        if (this->flags_[blk_pos]) {
            return false;
        }
        else {
            this->flags_[blk_pos] = true;
            return true;
        }
    }
    void unset(const blk_pos_type blk_pos)
    {
        MEFDN_ASSERT(this->flags_[blk_pos]);
        this->flags_[blk_pos] = false;
    }
    
private:
    mefdn::unique_ptr<bool []> flags_;
};

} // namespace medsm2
} // namespace menps

