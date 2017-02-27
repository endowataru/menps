
#include <mgdev/ibv/completion_queue.hpp>

namespace mgdev {
namespace ibv {

completion_queue::completion_queue()
    : cq_(MGBASE_NULLPTR) { }

completion_queue::~completion_queue()
{
    if (cq_ != MGBASE_NULLPTR)
        destroy();
}

void completion_queue::create(ibv_context& ctx)
{
    MGBASE_ASSERT(cq_ == MGBASE_NULLPTR);
    
    cq_ = ibv_create_cq(&ctx, num_cqe, MGBASE_NULLPTR, MGBASE_NULLPTR, 0);
    if (cq_ == MGBASE_NULLPTR)
        throw ibv_error("ibv_create_cq() failed");
}

void completion_queue::destroy() MGBASE_NOEXCEPT
{
    MGBASE_ASSERT(cq_ != MGBASE_NULLPTR);
    
    ibv_destroy_cq(cq_); // ignore error
    
    cq_ = MGBASE_NULLPTR;
}

} // namespace ibv
} // namespace mgdev
