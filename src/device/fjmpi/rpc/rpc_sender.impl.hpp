
#pragma once

#include "rpc_connection_pool.hpp"
#include "device/fjmpi/fjmpi.hpp"

namespace mgcom {
namespace rpc {

class rpc_sender
{
public:
    void initialize(rpc_connection_pool& pool)
    {
        pool_ = &pool;
    }
    
    bool try_call(
        const process_id_t          server_proc
    ,   const handler_id_t          handler_id
    ,   const void* const           arg_ptr
    ,   const index_t               arg_size
    ,   void* const                 return_ptr
    ,   const index_t               //return_size
    ,   const mgbase::operation&    on_complete
    ) {
        rpc_connection_pool::sender_info info;
        if (!pool_->try_start_sending(server_proc, &info))
            return false;
        
        message_buffer& request_buf = *info.local_buf;
        request_buf.is_reply    = false;
        request_buf.handler_id  = handler_id;
        request_buf.data_size   = arg_size;
        request_buf.return_ptr  = return_ptr;
        request_buf.on_complete = on_complete;
        
        send(server_proc, info, arg_ptr);
        
        return true;
    }
    
    bool try_send_reply(
        const process_id_t          client_proc
    ,   const message_buffer&       request_buf
    ,   const void* const           return_ptr
    ,   const index_t               return_size
    ) {
        rpc_connection_pool::sender_info info;
        if (!pool_->try_start_sending(client_proc, &info))
            return false;
        
        message_buffer& reply_buf = *info.local_buf;
        reply_buf.is_reply    = true;
        reply_buf.handler_id  = request_buf.handler_id; // not necessary (just for debugging)
        reply_buf.data_size   = return_size;
        reply_buf.return_ptr  = request_buf.return_ptr;
        reply_buf.on_complete = request_buf.on_complete;
        
        send(client_proc, info, return_ptr);
        
        return true;
    }

private:
    void send(
        const process_id_t                      dest_proc
    ,   const rpc_connection_pool::sender_info& info
    ,   const void* const                       data_ptr
    ) {
        message_buffer& local_buf = *info.local_buf;
        
        std::memcpy(local_buf.data, data_ptr, local_buf.data_size);
        
        const int flags = get_flags(info.remote_nic);
        
        while (!rma::untyped::try_remote_write_async_extra(
            dest_proc
        ,   info.remote_buf.to_address()
        ,   info.local_buf.to_address()
        ,   sizeof(message_buffer)
        ,   flags | FJMPI_RDMA_REMOTE_NOTICE
        ,   mgbase::make_no_operation()
        ))
        { }
        
        pool_->finish_sending(dest_proc, info.remote_nic);
    }
    
    int get_flags(const int remote_nic)
    {
        switch (remote_nic)
        {
            case 0: return FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH0;
            case 1: return FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH0;
            case 2: return FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH0;
            case 3: return FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH0;
            default: MGBASE_UNREACHABLE();
        }
    }
    
    rpc_connection_pool* pool_;
};

} // namespace rpc
} // namespace mgcom

