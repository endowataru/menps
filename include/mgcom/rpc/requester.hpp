
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rpc/call.h>
#include <mgbase/optional.hpp>
#include <mgbase/operation.hpp>

namespace mgcom {
namespace rpc {

struct constants
{
    static const index_t max_num_handlers = MGCOM_RPC_MAX_NUM_HANDLERS;
};

typedef mgcom_rpc_handler_parameters    handler_parameters;
typedef mgcom_rpc_handler_function_t    handler_function_t;
typedef mgcom_rpc_handler_id_t          handler_id_t;

namespace untyped {

struct register_handler_params
{
    handler_id_t            id;
    handler_function_t      callback;
};

struct call_params
{
    process_id_t        proc;
    handler_id_t        handler_id;
    const void*         arg_ptr;
    index_t             arg_size;
    void*               return_ptr;
    index_t             return_size;
    mgbase::operation   on_complete;
};

} // namespace untyped

class requester
    : mgbase::noncopyable
{
public:
    virtual ~requester() MGBASE_EMPTY_DEFINITION
    
    virtual void register_handler(const untyped::register_handler_params& params) = 0;
    
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

} // namespace rpc
} // namespace mgcom

