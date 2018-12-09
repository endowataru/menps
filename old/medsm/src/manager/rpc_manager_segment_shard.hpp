
#pragma once

#include "basic_rpc_manager_segment_shard.hpp"
#include "rpc_manager_page.hpp"
#include <menps/mecom/common_policy.hpp>
#include <menps/mecom/rpc/rpc_policy.hpp>
#include <menps/medsm/segment.hpp>

namespace menps {
namespace medsm {

class rpc_manager_segment_shard;

struct rpc_manager_segment_shard_policy
    : mecom::common_policy
    , mecom::rpc::rpc_policy
{
    typedef rpc_manager_segment_shard   derived_type;
    
    typedef rpc_manager_page            page_type;
    
    typedef page_id_t                   page_id_type;
};

class rpc_manager_segment_shard
    : public basic_rpc_manager_segment_shard<rpc_manager_segment_shard_policy>
{
    typedef basic_rpc_manager_segment_shard<rpc_manager_segment_shard_policy>   base;
    
public:
    template <typename Conf>
    explicit rpc_manager_segment_shard(const Conf& conf)
        : base(conf)
    { }
};

} // namespace medsm
} // namespace menps

