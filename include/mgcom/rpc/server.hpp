
#pragma once

#include <mgcom/rpc/message.hpp>
#include <mgbase/callback.hpp>

namespace mgcom {
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
    server_reply_message<void>      rply_msg;
};

typedef mgbase::callback<handler_result (handler_context_t)>
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
    server() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~server() MGBASE_DEFAULT_NOEXCEPT = default;
    
    server(const server&) = delete;
    server& operator = (const server&) = delete;
    
    virtual void add_handler(const untyped::add_handler_params&) = 0;
};

} // namespace rpc2
} // namespace mgcom

