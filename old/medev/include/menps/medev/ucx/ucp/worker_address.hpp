
#pragma once

#include <menps/medev/ucx/ucp/ucp.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct worker_address_deleter
{
    ucp_worker* wk;
    
    void operator () (ucp_address_t*) const noexcept;
};

class worker_address
    : public mefdn::unique_ptr<ucp_address_t [], worker_address_deleter>
{
    typedef mefdn::unique_ptr<ucp_address_t [], worker_address_deleter>  base;
    
public:
    worker_address() noexcept = default;
    
    explicit worker_address(ucp_worker* const wk, ucp_address_t* const p, const mefdn::size_t n)
        : base(p, worker_address_deleter{wk})
        , n_(n)
    { }
    
    worker_address(const worker_address&) = delete;
    worker_address& operator = (const worker_address&) = delete;
    
    worker_address(worker_address&&) noexcept = default;
    worker_address& operator = (worker_address&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return n_;
    }
    
private:
    mefdn::size_t  n_;
};

worker_address get_worker_address(ucp_worker* wk);

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

