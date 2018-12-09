
#pragma once

#include "basic_rpc_manager_space_shard.hpp"
#include "space_coherence_activater.hpp"
#include "rpc_manager_segment.hpp"
#include "manager_space.hpp"

namespace menps {
namespace medsm {

class rpc_manager_space;

struct rpc_manager_space_policy
{
    typedef rpc_manager_space           derived_type;
    
    typedef rpc_manager_segment         segment_shard_type;
    
    typedef segment_id_t                segment_id_type;
    
    typedef space_coherence_activater   activater_type;
};

class rpc_manager_space
    : public basic_rpc_manager_space_shard<rpc_manager_space_policy>
{
    typedef basic_rpc_manager_space_shard<rpc_manager_space_policy> base;
    
public:
    template <typename Conf>
    explicit rpc_manager_space(const Conf& conf)
        : base(conf)
        , activater_(nullptr)
    { }
    
    inline rpc_manager_segment::accessor get_segment_accessor(segment_id_t);
    
    class proxy;
    
    inline proxy make_proxy_collective();
    
    space_coherence_activater& get_activater()
    {
        MEFDN_ASSERT(activater_ != nullptr);
        return *activater_;
    }
    void set_activater(space_coherence_activater& activater)
    {
        MEFDN_ASSERT(activater_ == nullptr);
        activater_ = &activater;
    }
    
private:
    space_coherence_activater*      activater_;
};

} // namespace medsm
} // namespace menps

