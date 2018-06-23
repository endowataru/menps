
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/medev2/ucx/ucs/ucs.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct worker_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator () (uct_worker* const p) const noexcept {
        uf->worker_destroy({ p });
    }
};

template <typename P>
class worker
    : public mefdn::unique_ptr<uct_worker, worker_deleter<P>>
{
    using deleter_type = worker_deleter<P>;
    using base = mefdn::unique_ptr<uct_worker, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    worker() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit worker(uct_facade_type& uf, uct_worker* const p)
        : base(p, deleter_type{ &uf })
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    worker(worker&&) noexcept = default;
    worker& operator = (worker&&) noexcept = default;
    
    static worker create(
        uct_facade_type&            uf
    ,   ucs_async_context_t* const  async
    ,   const ucs_thread_mode_t     thread_mode
    ) {
        uct_worker* p = nullptr;
        
        const auto ret = uf.worker_create({ async, thread_mode, &p });
        if (ret != UCS_OK) {
            throw ucx_error("uct_worker_create() failed", ret);
        }
        
        return worker(uf, p);
    }
    
    unsigned int progress()
    {
        auto& uf = * this->get_deleter().uf;
        
        MEFDN_LOG_VERBOSE("msg:Call uct_worker_progress().");
        
        const auto ret = uf.worker_progress({ this->get() });
        return ret;
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

