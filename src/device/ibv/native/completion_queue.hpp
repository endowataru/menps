
#pragma once

#include "verbs.hpp"
#include <mgbase/assert.hpp>
#include "device/ibv/ibv_error.hpp"

namespace mgcom {
namespace ibv {

class completion_queue
    : mgbase::noncopyable
{
    static const mgbase::uint32_t num_cqe = 1024;
    
public:
    completion_queue()
        : cq_(MGBASE_NULLPTR) { }
    
    ~completion_queue() {
        if (cq_ != MGBASE_NULLPTR)
            destroy();
    }
    
    void create(ibv_context& ctx)
    {
        MGBASE_ASSERT(cq_ == MGBASE_NULLPTR);
        
        cq_ = ibv_create_cq(&ctx, num_cqe, MGBASE_NULLPTR, MGBASE_NULLPTR, 0);
        if (cq_ == MGBASE_NULLPTR)
            throw ibv_error("ibv_create_cq() failed");
    }
    
    void destroy() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(cq_ != MGBASE_NULLPTR);
        
        ibv_destroy_cq(cq_); // ignore error
        
        cq_ = MGBASE_NULLPTR;
    }
    
    int poll(
        ibv_wc* const   wc_array
    ,   const int       num_entries
    ) {
        MGBASE_ASSERT(cq_ != MGBASE_NULLPTR);
        
        const int ret = ibv_poll_cq(cq_, num_entries, wc_array);
        if (ret < 0)
            throw ibv_error("ibv_poll_cq() failed", ret);
        
        return ret;
    }
    
    ibv_cq& get() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(cq_ != MGBASE_NULLPTR);
        return *cq_;
    }
    
private:
    ibv_cq* cq_;
};

} // namespace ibv
} // namespace mgcom

