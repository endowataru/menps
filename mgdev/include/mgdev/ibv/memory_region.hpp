
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ibv {

struct memory_region_deleter
{
    void operator () (ibv_mr*) const MGBASE_NOEXCEPT;
};

class memory_region
    : public mgbase::unique_ptr<ibv_mr, memory_region_deleter>
{
    typedef mgbase::unique_ptr<ibv_mr, memory_region_deleter>  base;
    
public:
    memory_region() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit memory_region(ibv_mr* const mr)
        : base(mr)
    { }
    
    ~memory_region() /*noexcept*/ = default;
    
    memory_region(const memory_region&) = delete;
    memory_region& operator = (const memory_region&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(memory_region, base)
};

memory_region make_memory_region(
    ibv_pd*         pd
,   void*           ptr
,   mgbase::size_t  size_in_bytes
,   int             access =
        IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
        IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC
);

} // namespace ibv
} // namespace mgdev

