
#pragma once

#include "rpc_base.hpp"
#include "rpc_server_thread.hpp"

namespace mgcom {
namespace mpi {

class rpc_server
    : public virtual rpc_base
{
    static const index_t num_threads = 1;
    
    typedef mgbase::unique_ptr<rpc_server_thread>   server_thread_ptr;
    
public:
    // TODO: adjustable (?)
    static const int request_tag = 100;
    
    explicit rpc_server(mpi_interface& mi)
        : invoker_{}
    {
        ths_ = mgbase::make_unique<server_thread_ptr []>(num_threads);
        
        const auto comm = this->get_comm();
        
        for (index_t i = 0; i < num_threads; ++i) {
            ths_[i] =
                mgbase::make_unique<rpc_server_thread>(
                    rpc_server_thread::config{
                        &mi
                    ,   &invoker_
                    ,   comm
                    ,   request_tag
                    }
                );
        }
    }
    
    ~rpc_server() = default;
    
    virtual void register_handler(const rpc::untyped::register_handler_params& params) MGBASE_OVERRIDE
    {
        invoker_.register_handler(params);
    }
    
private:
    rpc::rpc_invoker    invoker_;
    
    mgbase::unique_ptr<server_thread_ptr []> ths_;
};

} // namespace mpi
} // namespace mgcom

