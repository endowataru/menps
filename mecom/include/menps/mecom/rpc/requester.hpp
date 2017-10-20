
#pragma once

#include <menps/mecom/common.hpp>
#include <menps/mecom/rpc/client.hpp>
#include <menps/mecom/rpc/server.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace mecom {
namespace rpc {

class requester
    : public client
    , public server
{
protected:
    requester() noexcept = default;
    
public:
    // Important: Virtual "default" destructors are not properly called
    //            in GCC 4.4 and causes a runtime error.
    virtual ~requester() noexcept { }
    
    static requester& get_instance() noexcept {
        MEFDN_ASSERT(req_ != nullptr);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};

} // namespace rpc
} // namespace mecom
} // namespace menps

