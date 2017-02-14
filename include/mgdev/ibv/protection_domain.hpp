
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/logger.hpp>

namespace mgdev {
namespace ibv {

class protection_domain
{
public:
    protection_domain()
        : pd_(MGBASE_NULLPTR) { }
    
    ~protection_domain()
    {
        if (pd_ != MGBASE_NULLPTR)
            dealloc();
    }
    
    protection_domain(const protection_domain&) = delete;
    protection_domain& operator = (const protection_domain&) = delete;
    
    void alloc(ibv_context& ctx)
    {
        MGBASE_ASSERT(pd_ == MGBASE_NULLPTR);
        
        pd_ = ibv_alloc_pd(&ctx);
        if (pd_ == MGBASE_NULLPTR)
            throw ibv_error("ibv_alloc_pd() failed");
    }
    
    void dealloc() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(pd_ != MGBASE_NULLPTR);
        
        ibv_dealloc_pd(pd_);
        
        pd_ = MGBASE_NULLPTR;
    }
    
    ibv_mr& register_memory(
        void* const             buf
    ,   const mgbase::size_t    size_in_bytes
    ) {
        MGBASE_ASSERT(pd_ != MGBASE_NULLPTR);
        
        ibv_mr* const mr = ibv_reg_mr(pd_, buf, size_in_bytes, 
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
            IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC);
        
        if (mr == MGBASE_NULLPTR)
            throw ibv_error("ibv_reg_mr() failed");
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region.\t"
            "ptr:{:x}\tsize_in_bytes:{}\t"
            "lkey:{:x}\trkey:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   size_in_bytes
        ,   mr->lkey
        ,   mr->rkey
        );
        
        return *mr;
    }
    
    static void deregister_memory(ibv_mr& mr) MGBASE_NOEXCEPT
    {
        ibv_dereg_mr(&mr); // ignore error
    }
    
    ibv_pd& get() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(pd_ != MGBASE_NULLPTR);
        return *pd_;
    }
    
private:
    ibv_pd* pd_;
};

} // namespace ibv
} // namespace mgdev

