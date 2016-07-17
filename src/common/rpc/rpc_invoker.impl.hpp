
#pragma once

#include <mgcom/rpc.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logging/logger.hpp>
#include <vector>

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

class rpc_invoker
{
    struct handler
    {
        handler_function_t  callback;
        void*               ptr;
    };
    
public:
    rpc_invoker()
        : handlers_(constants::max_num_handlers)
    {
        /*// Zero-fill by value initialization
        handlers_ = new handler[]{};*/
    }
    
    void register_handler(const untyped::register_handler_params& params)
    {
        MGBASE_ASSERT(params.id < constants::max_num_handlers);
        MGBASE_ASSERT(params.callback != MGBASE_NULLPTR);
        
        handlers_[params.id] = handler{ params.callback, params.ptr };
        
        MGBASE_LOG_DEBUG(
            "msg:Registered a handler.\t"
            "src:{}\tcallback:{:x}\tptr:{:x}"
        ,   params.id
        ,   reinterpret_cast<mgbase::uintptr_t>(params.callback)
        ,   reinterpret_cast<mgbase::uintptr_t>(params.ptr)
        );
    }
    
    index_t call(
        const process_id_t          src_proc
    ,   const handler_id_t          handler_id
    ,   const void* const           arg_data
    ,   const index_t               arg_size
    ,   void* const                 result_data
    ,   const index_t               result_capacity
    ) {
        const handler_parameters params = {
            src_proc
        ,   arg_data
        ,   arg_size
        ,   result_data
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Invoking callback.\t"
            "src:{}\tid:{}"
        ,   src_proc
        ,   handler_id
        );
        
        const auto& h = handlers_[handler_id];
        MGBASE_ASSERT(h.callback != MGBASE_NULLPTR);
        
        const index_t reply_size = h.callback(h.ptr, &params);
        
        MGBASE_ASSERT(reply_size <= result_capacity);
        
        MGBASE_LOG_DEBUG("msg:Finished callback.");
        
        return reply_size;
    }
    
private:
    std::vector<handler> handlers_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mgcom

