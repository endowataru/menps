
#pragma once

#include <mgcom/rpc/message.hpp>
#include <mgbase/callback.hpp>

namespace mgcom {
namespace rpc {

namespace untyped {

typedef mgbase::callback<void (client_reply_message<void>&&)> call_callback_t;

struct async_call_params
{
    process_id_t                    server_proc;
    handler_id_t                    handler_id;
    client_request_message<void>&&  rqst_msg;
    call_callback_t                 on_complete;
};

} // namespace untyped

class client
{
protected:
    client() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~client() MGBASE_DEFAULT_NOEXCEPT = default;
    
    client(const client&) = delete;
    client& operator = (const client&) = delete;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<client_reply_message<void>>
    async_call(
        const untyped::async_call_params&
    ) = 0;
};

} // namespace rpc
} // namespace mgcom

