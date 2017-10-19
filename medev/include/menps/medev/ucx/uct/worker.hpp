
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct worker_deleter
{
    void operator () (uct_worker*) const noexcept;
};

class worker
    : public mefdn::unique_ptr<uct_worker, worker_deleter>
{
    typedef mefdn::unique_ptr<uct_worker, worker_deleter>  base;
    
public:
    worker() noexcept = default;
    
    explicit worker(uct_worker* const p)
        : base(p)
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    worker(worker&&) noexcept = default;
    worker& operator = (worker&&) noexcept = default;
    
    void progress();
};

worker create_worker(
    ucs_async_context_t*    async
,   ucs_thread_mode_t       thread_mode
);

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

