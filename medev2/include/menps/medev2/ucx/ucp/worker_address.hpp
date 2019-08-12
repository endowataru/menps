
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct worker_address_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type*    uf;
    ucp_worker*         wk;
    
    void operator () (ucp_address_t* const p) const noexcept {
        this->uf->worker_release_address({ this->wk, p });
    }
};

template <typename P>
class worker_address
    : public mefdn::unique_ptr<ucp_address_t, worker_address_deleter<P>>
{
    using deleter_type = worker_address_deleter<P>;
    using base = mefdn::unique_ptr<ucp_address_t, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    worker_address() noexcept = default;
    
    explicit worker_address(
        ucp_facade_type&        uf
    ,   ucp_worker* const       wk
    ,   ucp_address_t* const    p
    ,   const mefdn::size_t     n
    )
        : base(p, deleter_type{ &uf, wk })
        , n_(n)
    { }
    
    worker_address(const worker_address&) = delete;
    worker_address& operator = (const worker_address&) = delete;
    
    worker_address(worker_address&&) noexcept = default;
    worker_address& operator = (worker_address&&) noexcept = default;
    
    mefdn::size_t size_in_bytes() const noexcept {
        return this->n_;
    }
    
    static worker_address get_address(
        ucp_facade_type&    uf
    ,   ucp_worker* const   wk
    ) {
        ucp_address_t* p = nullptr;
        mefdn::size_t size = 0;
        
        const auto ret = uf.worker_get_address({ wk, &p, &size });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_worker_get_address() failed", ret);
        }
        
        return worker_address(uf, wk, p, size);
    }
    
private:
    mefdn::size_t  n_;
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

