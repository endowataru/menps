
#pragma once

#include "verbs.hpp"
#include <mgbase/assert.hpp>
#include "device/ibv/ibv_error.hpp"

namespace mgcom {
namespace ibv {

class context
    : mgbase::noncopyable
{
public:
    context()
        : ctx_(MGBASE_NULLPTR) { }
    
    ~context()
    {
        if (ctx_ != MGBASE_NULLPTR)
            close();
    }
    
    void open(ibv_device& dev)
    {
        MGBASE_ASSERT(ctx_ == MGBASE_NULLPTR);
        
        ctx_ = ibv_open_device(&dev);
        if (ctx_ == MGBASE_NULLPTR)
            throw ibv_error("ibv_open_device() failed");
    }
    
    void close() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(ctx_ != MGBASE_NULLPTR);
        
        ibv_close_device(ctx_); // ignore error
        
        ctx_ = MGBASE_NULLPTR;
    }
    
    ibv_context& get() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(ctx_ != MGBASE_NULLPTR);
        return *ctx_;
    }
    
private:
    ibv_context* ctx_;
};

} // namespace ibv
} // namespace mgcom

