
#pragma once

#include "rpc_manager_segment_shard.hpp"

namespace mgdsm {

class rpc_manager_segment
    : private rpc_manager_segment_shard
{
public:
    class accessor;
    
    class proxy;
};

typedef mgbase::unique_ptr<rpc_manager_segment> rpc_manager_segment_ptr;

} // namespace mgdsm

