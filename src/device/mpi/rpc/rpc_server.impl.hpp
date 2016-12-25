
#pragma once

#include "rpc_server_thread.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_server
    : mgbase::noncopyable
{
    static const index_t num_threads = 1;
    
    static const int tag = 100/*FIXME*/;
    
    typedef mgbase::unique_ptr<rpc_server_thread>   server_thread_ptr;
    
public:
    explicit rpc_server(mpi_interface& mi)
        : invoker_{}
    {
        comm_ = mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_RPC");
        
        ths_ = mgbase::make_unique<server_thread_ptr []>(num_threads);
        
        for (index_t i = 0; i < num_threads; ++i) {
            ths_[i] =
                mgbase::make_unique<rpc_server_thread>(
                    rpc_server_thread::conf{
                        &mi
                    ,   &invoker_
                    ,   comm_
                    ,   tag
                    }
                );
        }
    }
    
    ~rpc_server() = default;
    
    MPI_Comm get_comm() const MGBASE_NOEXCEPT {
        return comm_;
    }
    
    void register_handler(const untyped::register_handler_params& params)
    {
        invoker_.register_handler(params);
    }
    
private:
    MPI_Comm    comm_;
    rpc_invoker invoker_;
    
    mgbase::unique_ptr<server_thread_ptr []> ths_;
};

} // unnamed namespace

} // namespace rpc
} // namespace mpi
} // namespace mgcom

