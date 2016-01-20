
#pragma once

#include <mgcom.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

class basic_receiver
{
public:
    void register_handler(handler_id_t id, handler_callback_t callback)
    {
        MGBASE_ASSERT(id < constants::max_num_callbacks);
        MGBASE_ASSERT(callback != MGBASE_NULLPTR);
        
        callbacks_[id] = callback;
        MGBASE_LOG_DEBUG("msg:Registered a handler.\tsrc:{}\tcallback:{:x}", id, reinterpret_cast<mgbase::uint64_t>(callback));
    }
    
protected:
    void initialize(index_t max_num_buffers)
    {
        callbacks_ = new handler_callback_t[constants::max_num_callbacks];
        buffers_ = new am_message_buffer[max_num_buffers];
        max_num_buffers_ = max_num_buffers;
    }
    
    void finalize()
    {
        callbacks_.reset();
    }
    
    void call(process_id_t src, am_message_buffer& msg) {
        callback_parameters params;
        params.source = src;
        params.data   = msg.data;
        params.size   = msg.size;
        
        MGBASE_LOG_DEBUG("msg:Invoking callback.\tsrc:{}\tid:{}", src, msg.id);
        
        const handler_callback_t callback = callbacks_[msg.id];
        MGBASE_ASSERT(callback != MGBASE_NULLPTR);
        callback(&params);
        
        MGBASE_LOG_DEBUG("msg:Finished callback.");
    }
    
    am_message_buffer& get_buffer_at(index_t index) {
        MGBASE_ASSERT(index < max_num_buffers_);
        return buffers_[index];
    }
    
private:
    mgbase::scoped_ptr<am_message_buffer []>    buffers_;
    mgbase::scoped_ptr<handler_callback_t []>   callbacks_;
    index_t max_num_buffers_;
};

} // unnamed namespace

} // namespace am
} // namespace mgcom

