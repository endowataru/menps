
#pragma once

#include "rpc_manager_segment_shard.hpp"

namespace mgdsm {

class rpc_manager_segment
    : private rpc_manager_segment_shard
{
    typedef rpc_manager_segment_shard   base;
    
public:
    template <typename Conf>
    explicit rpc_manager_segment(const Conf& conf)
        : base(conf)
    { }
    
    class accessor;
    
    class proxy;
};

typedef mgbase::unique_ptr<rpc_manager_segment> rpc_manager_segment_ptr;

} // namespace mgdsm

