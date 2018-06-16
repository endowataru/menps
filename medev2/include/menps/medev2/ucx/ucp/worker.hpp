
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct worker_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type* uf;
    
    void operator() (ucp_worker* const wk) {
        uf->worker_destroy({ wk });
    }
};

template <typename P>
class worker
    : public mefdn::unique_ptr<ucp_worker, worker_deleter<P>>
{
    using deleter_type = worker_deleter<P>;
    using base = mefdn::unique_ptr<ucp_worker, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    worker() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit worker(ucp_facade_type& uf, ucp_worker* const wk)
        : base(wk, deleter_type{ &uf })
    { }
    
    static worker create(
        ucp_facade_type&                    uf
    ,   ucp_context* const                  ctx
    ,   const ucp_worker_params_t* const    params
    ) {
        ucp_worker* p = nullptr;
        
        const auto ret = uf.worker_create({ ctx, params, &p });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_worker_create() failed", ret);
        }
        
        return worker(uf, p);
    }
    
    unsigned int progress(ucp_facade_type& uf)
    {
        auto wk = this->get();
        return uf.worker_progress({ wk });
    }
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

