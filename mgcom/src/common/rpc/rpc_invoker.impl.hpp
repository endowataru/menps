
#pragma once

#include <mgcom/rpc.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logging/logger.hpp>
#include <vector>

namespace mgcom {
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
        MGBASE_ASSERT(params.id < constants::max_num_handlers);
        MGBASE_ASSERT(params.cb);
        
        handlers_[params.id] = handler{ params.cb };
        
        MGBASE_LOG_DEBUG(
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
        const untyped::handler_context hc{ src_proc, mgbase::move(rqst_msg) };
        
        MGBASE_LOG_DEBUG(
            "msg:Invoking callback.\t"
            "handler_id:{}\t"
            "src:{}"
        ,   handler_id
        ,   src_proc
        );
        
        const auto& h = handlers_[handler_id];
        MGBASE_ASSERT(h.cb);
        
        untyped::handler_result r{ h.cb(hc) };
        
        MGBASE_LOG_DEBUG("msg:Finished callback.");
        
        return r;
    }
    
private:
    std::vector<handler> handlers_;
};

} // namespace rpc
} // namespace mgcom
