
#pragma once

#include <mgdev/ucx/ucp/worker_address.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct worker_deleter
{
    void operator () (ucp_worker*) const MGBASE_NOEXCEPT;
};

class worker
    : public mgbase::unique_ptr<ucp_worker, worker_deleter>
{
    typedef mgbase::unique_ptr<ucp_worker, worker_deleter>  base;
    
public:
    worker() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit worker(ucp_worker* const p)
        : base(p)
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(worker, base)
    
    worker_address get_address();
    
    void progress();
};

worker create_worker(
    ucp_context*                ctx
,   const ucp_worker_params_t*  params
);

} // namespace ucp
} // namespace ucx
} // namespace mgdev

