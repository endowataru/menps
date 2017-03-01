
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>

namespace mgdev {
namespace ibv {

completion_queue make_completion_queue(ibv_context* const ctx, const int num_cqe)
{
    const auto cq = ibv_create_cq(ctx, num_cqe, MGBASE_NULLPTR, MGBASE_NULLPTR, 0);
    if (cq == MGBASE_NULLPTR)
        throw ibv_error("ibv_create_cq() failed");
    
    return completion_queue(cq);
}

void completion_queue::poll_error(const int err)
{
    throw ibv_error("ibv_poll_cq() failed", err);
}

void completion_queue_deleter::operator () (ibv_cq* const cq) const MGBASE_NOEXCEPT
{
    if (cq == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_destroy_cq(cq); // ignore error
}

} // namespace ibv
} // namespace mgdev
