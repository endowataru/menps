
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct worker_deleter
{
    void operator () (uct_worker*) const MGBASE_NOEXCEPT;
};

class worker
    : public mgbase::unique_ptr<uct_worker, worker_deleter>
{
    typedef mgbase::unique_ptr<uct_worker, worker_deleter>  base;
    
public:
    worker() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit worker(uct_worker* const p)
        : base(p)
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(worker, base)
    
    void progress();
};

worker create_worker(
    ucs_async_context_t*    async
,   ucs_thread_mode_t       thread_mode
);

} // namespace uct
} // namespace ucx
} // namespace mgdev

