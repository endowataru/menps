
#pragma once

#include <menps/mecom/rpc/message.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace mecom {
namespace rpc {

namespace untyped {

struct handler_context
{
    process_id_t                    src_proc;
    server_request_message<void>&&  rqst_msg;
};

typedef const handler_context&  handler_context_t;

struct handler_result
{
    // These members are only needed in GCC 4.x.
    #if 0
    handler_result() noexcept = default;
    
    /*implicit*/ handler_result(server_reply_message<void> msg)
        : rply_msg(mefdn::move(msg))
    { }
    
    handler_result(const handler_result&) = delete;
    handler_result& operator = (const handler_result&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_1(handler_result, rply_msg)
    #endif
    
    server_reply_message<void>      rply_msg;
};

typedef mefdn::callback<handler_result (handler_context_t)>
    handler_callback_t;

struct add_handler_params
{
    handler_id_t        id;
    handler_callback_t  cb;
};

} // namespace untyped

class server
{
protected:
    server() noexcept = default;
    
public:
    virtual ~server() noexcept = default;
    
    server(const server&) = delete;
    server& operator = (const server&) = delete;
    
    virtual void add_handler(const untyped::add_handler_params&) = 0;
};

} // namespace rpc2
} // namespace mecom
} // namespace menps

