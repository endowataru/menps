
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/assert.hpp>
#include <mgdev/ibv/ibv_error.hpp>

namespace mgdev {
namespace ibv {

class completion_queue
{
    static const mgbase::uint32_t num_cqe = 1 << 18;
    
public:
    completion_queue()
        : cq_(MGBASE_NULLPTR) { }
    
    ~completion_queue() {
        if (cq_ != MGBASE_NULLPTR)
            destroy();
    }
    
    completion_queue(const completion_queue&) = delete;
    completion_queue& operator =  (const completion_queue&) = delete;
    
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
        if (MGBASE_UNLIKELY(ret < 0))
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
} // namespace mgdev

