
#pragma once

#include <mgcom/rpc.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

class rpc_invoker
{
public:
    void initialize()
    {
        callbacks_ = new handler_function_t[constants::max_num_handlers];
    }
    
    void finalize()
    {
        callbacks_.reset();
    }
    
    void register_handler(handler_id_t id, handler_function_t callback)
    {
        MGBASE_ASSERT(id < constants::max_num_handlers);
        MGBASE_ASSERT(callback != MGBASE_NULLPTR);
        
        callbacks_[id] = callback;
        MGBASE_LOG_DEBUG("msg:Registered a handler.\tsrc:{}\tcallback:{:x}", id, reinterpret_cast<mgbase::uint64_t>(callback));
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
        
        MGBASE_LOG_DEBUG("msg:Invoking callback.\tsrc:{}\tid:{}", src_proc, handler_id);
        
        const handler_function_t callback = callbacks_[handler_id];
        MGBASE_ASSERT(callback != MGBASE_NULLPTR);
        
        const index_t reply_size = callback(&params);
        
        MGBASE_ASSERT(reply_size <= result_capacity);
        
        MGBASE_LOG_DEBUG("msg:Finished callback.");
        
        return reply_size;
    }
    
private:
    mgbase::scoped_ptr<handler_function_t []>   callbacks_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mgcom

