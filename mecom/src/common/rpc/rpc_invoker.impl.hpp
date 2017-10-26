
#pragma once

#include <menps/mecom/rpc.hpp>
#include <menps/mefdn/logger.hpp>
#include <vector>

namespace menps {
namespace mecom {
namespace rpc {

class rpc_invoker
{
    struct handler
    {
        untyped::handler_callback_t  cb;
    };
    
public:
    rpc_invoker()
        : handlers_(constants::max_num_handlers)
    {
        /*// Zero-fill by value initialization
        handlers_ = new handler[]{};*/
    }
    
    void add_handler(const untyped::add_handler_params& params)
    {
        MEFDN_ASSERT(params.id < constants::max_num_handlers);
        MEFDN_ASSERT(params.cb);
        
        handlers_[params.id] = handler{ params.cb };
        
        MEFDN_LOG_DEBUG(
            "msg:Registered a handler.\t"
            "handler_id:{}"
        ,   params.id
        );
    }
    
    /*index_t call(
        const process_id_t          src_proc
    ,   const handler_id_t          handler_id
    ,   const void* const           arg_data
    ,   const index_t               arg_size
    ,   void* const                 result_data
    ,   const index_t               result_capacity
    )*/
    untyped::handler_result
    call(
        handler_id_t                    handler_id
    ,   process_id_t                    src_proc
    ,   server_request_message<void>    rqst_msg
    ) {
        const untyped::handler_context hc{ src_proc, mefdn::move(rqst_msg) };
        
        MEFDN_LOG_DEBUG(
            "msg:Invoking callback.\t"
            "handler_id:{}\t"
            "src:{}"
        ,   handler_id
        ,   src_proc
        );
        
        const auto& h = handlers_[handler_id];
        MEFDN_ASSERT(h.cb);
        
        auto r = h.cb(hc);
        
        MEFDN_LOG_DEBUG("msg:Finished callback.");
        
        return r;
    }
    
private:
    std::vector<handler> handlers_;
};

} // namespace rpc
} // namespace mecom
} // namespace menps

