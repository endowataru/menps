
#pragma once

#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_rpc_manager_space_shard
    : public mefdn::crtp_base<Policy>
{
    typedef typename Policy::derived_type           derived_type;
    
    typedef typename Policy::segment_shard_type     segment_shard_type;
    
    typedef typename Policy::segment_id_type        segment_id_type;
    
protected:
    template <typename Conf>
    explicit basic_rpc_manager_space_shard(const Conf& conf)
        : segs_(
            mefdn::make_unique<mefdn::unique_ptr<segment_shard_type> []>(conf.num_segments)
        )
    { }
    
public:
    segment_shard_type& get_segment(const segment_id_type seg_id)
    {
        MEFDN_ASSERT(segs_[seg_id] != nullptr);
        
        return *segs_[seg_id];
    }
    
    template <typename Conf>
    void make_segment(const segment_id_type seg_id, const Conf& conf)
    {
        MEFDN_ASSERT(segs_[seg_id] == nullptr);
        
        segs_[seg_id] = mefdn::make_unique<segment_shard_type>(conf);
    }
    
private:
    mefdn::unique_ptr<mefdn::unique_ptr<segment_shard_type> []>   segs_;
};

} // namespace medsm
} // namespace menps

