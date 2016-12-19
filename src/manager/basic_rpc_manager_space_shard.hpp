
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_space_shard
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type           derived_type;
    
    typedef typename Policy::segment_shard_type     segment_shard_type;
    
    typedef typename Policy::segment_id_type        segment_id_type;
    
public:
    segment_shard_type& get_segment(const segment_id_t seg_id)
    {
        const auto index = seg_id / Policy::number_of_processes();
        
        return *segs_[index];
    }
    
    template <typename Conf>
    void make_segment(const segment_id_type seg_id, const Conf& conf)
    {
        MGBASE_ASSERT(segs_[seg_id] == MGBASE_NULLPTR);
        
        segs_[seg_id] = mgbase::make_unique<segment_shard_type>(conf);
    }
    
private:
    
    mgbase::unique_ptr<mgbase::unique_ptr<segment_shard_type> []>   segs_;
};

} // namespace mgdsm

