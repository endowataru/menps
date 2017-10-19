
#pragma once

#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ibv {

struct memory_region_deleter
{
    void operator () (ibv_mr*) const noexcept;
};

class memory_region
    : public mefdn::unique_ptr<ibv_mr, memory_region_deleter>
{
    typedef mefdn::unique_ptr<ibv_mr, memory_region_deleter>  base;
    
public:
    memory_region() noexcept = default;
    
    explicit memory_region(ibv_mr* const mr)
        : base(mr)
    { }
    
    ~memory_region() /*noexcept*/ = default;
    
    memory_region(const memory_region&) = delete;
    memory_region& operator = (const memory_region&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(memory_region, base)
};

memory_region make_memory_region(
    ibv_pd*         pd
,   void*           ptr
,   mefdn::size_t  size_in_bytes
,   int             access =
        IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
        IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC
);

} // namespace ibv
} // namespace medev
} // namespace menps

