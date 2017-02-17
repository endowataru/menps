
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rpc/client.hpp>
#include <mgcom/rpc/server.hpp>
#include <mgcom/rpc/call.h>
#include <mgbase/callback.hpp>

namespace mgcom {
namespace rpc {

class requester
    : public client
    , public server
{
public:
    static requester& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(req_ != MGBASE_NULLPTR);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};

#if 0

typedef mgcom_rpc_handler_parameters    handler_parameters;
typedef mgcom_rpc_handler_function_t    handler_function_t;
typedef mgcom_rpc_handler_id_t          handler_id_t;

namespace untyped {

struct register_handler_params
{
    handler_id_t        id;
    handler_function_t  callback;
    void*               ptr;
};

struct call_params
{
    process_id_t                proc;
    handler_id_t                handler_id;
    const void*                 arg_ptr;
    index_t                     arg_size;
    void*                       return_ptr;
    index_t                     return_size;
    mgbase::callback<void ()>   on_complete;
};

} // namespace untyped

class requester
{
public:
    virtual ~requester() MGBASE_EMPTY_DEFINITION
    
    requester(const requester&) = delete;
    requester& operator = (const requester&) = delete;
    
    virtual void register_handler(const untyped::register_handler_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_call(const async_untyped_call_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_call_async(const untyped::call_params& params) = 0;
    
    static requester& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(req_ != MGBASE_NULLPTR);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};

#endif

} // namespace rpc
} // namespace mgcom

