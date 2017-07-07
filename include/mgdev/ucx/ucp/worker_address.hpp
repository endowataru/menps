
#pragma once

#include <mgdev/ucx/ucp/ucp.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct worker_address_deleter
{
    ucp_worker* wk;
    
    void operator () (ucp_address_t*) const MGBASE_NOEXCEPT;
};

class worker_address
    : public mgbase::unique_ptr<ucp_address_t [], worker_address_deleter>
{
    typedef mgbase::unique_ptr<ucp_address_t [], worker_address_deleter>  base;
    
public:
    worker_address() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit worker_address(ucp_worker* const wk, ucp_address_t* const p, const mgbase::size_t n)
        : base(p, worker_address_deleter{wk})
        , n_(n)
    { }
    
    worker_address(const worker_address&) = delete;
    worker_address& operator = (const worker_address&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(worker_address, base, n_)
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return n_;
    }
    
private:
    mgbase::size_t  n_;
};

worker_address get_worker_address(ucp_worker* wk);

} // namespace ucp
} // namespace ucx
} // namespace mgdev

