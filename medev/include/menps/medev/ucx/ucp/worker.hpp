
#pragma once

#include <menps/medev/ucx/ucp/worker_address.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct worker_deleter
{
    void operator () (ucp_worker*) const noexcept;
};

class worker
    : public mefdn::unique_ptr<ucp_worker, worker_deleter>
{
    typedef mefdn::unique_ptr<ucp_worker, worker_deleter>  base;
    
public:
    worker() noexcept = default;
    
    explicit worker(ucp_worker* const p)
        : base(p)
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(worker, base)
    
    worker_address get_address();
    
    void progress();
};

worker create_worker(
    ucp_context*                ctx
,   const ucp_worker_params_t*  params
);

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

