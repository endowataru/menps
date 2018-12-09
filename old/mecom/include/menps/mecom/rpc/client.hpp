
#pragma once

#include <menps/mecom/rpc/message.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace mecom {
namespace rpc {

namespace untyped {

typedef mefdn::callback<void (client_reply_message<void>&&)> call_callback_t;

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
    client() noexcept = default;
    
public:
    virtual ~client() noexcept = default;
    
    client(const client&) = delete;
    client& operator = (const client&) = delete;
    
    MEFDN_NODISCARD
    virtual ult::async_status<client_reply_message<void>>
    async_call(
        const untyped::async_call_params&
    ) = 0;
};

} // namespace rpc
} // namespace mecom
} // namespace menps

