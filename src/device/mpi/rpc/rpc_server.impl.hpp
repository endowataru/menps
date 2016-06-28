
#pragma once

#include "rpc_server_thread.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_server
    : mgbase::noncopyable
{
    static const index_t num_threads = 1;
    
public:
    explicit rpc_server(mpi_interface& mi)
    {
        invoker_.initialize();
        
        comm_ = mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_RPC");
        
        ths_ = new rpc_server_thread[num_threads];
        
        for (index_t i = 0; i < num_threads; ++i)
            ths_[i].initialize(mi, invoker_, comm_, 100/*FIXME*/);
    }
    
    ~rpc_server()
    {
        for (index_t i = 0; i < num_threads; ++i)
            ths_[i].finalize();
        
        ths_.reset();
        
        invoker_.finalize();
    }
    
    MPI_Comm get_comm() const MGBASE_NOEXCEPT {
        return comm_;
    }
    
    void register_handler(const handler_id_t id, const handler_function_t callback)
    {
        invoker_.register_handler(id, callback);
    }
    
private:
    MPI_Comm comm_;
    rpc_invoker invoker_;
    mgbase::scoped_ptr<rpc_server_thread []> ths_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mpi
} // namespace mgcom

