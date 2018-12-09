
#include <menps/medev/ibv/completion_queue.hpp>
#include <menps/medev/ibv/attributes.hpp>
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medev {
namespace ibv {

completion_queue make_completion_queue(ibv_context* const ctx, const int num_cqe)
{
    const auto cq = ibv_create_cq(ctx, num_cqe, nullptr, nullptr, 0);
    if (cq == nullptr)
        throw ibv_error("ibv_create_cq() failed");
    
    return completion_queue(cq);
}

completion_queue make_completion_queue(ibv_context* const ctx)
{
    const auto dev_attr = query_device(ctx);
    
    return make_completion_queue(ctx, dev_attr.max_cqe);
}

void completion_queue::poll_error(const int err)
{
    throw ibv_error("ibv_poll_cq() failed", err);
}

void completion_queue_deleter::operator () (ibv_cq* const cq) const noexcept
{
    if (cq == nullptr) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_destroy_cq(cq); // ignore error
}

} // namespace ibv
} // namespace medev
} // namespace menps

