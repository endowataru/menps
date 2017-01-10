
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
    explicit rpc_server(mpi_interface& mi)
        : rpc_base(mi)
    {
        ths_ = mgbase::make_unique<server_thread_ptr []>(num_threads);
        
        const auto comm = this->get_comm();
        
        const auto tag = this->get_server_tag();
        
        for (index_t i = 0; i < num_threads; ++i) {
            ths_[i] =
                mgbase::make_unique<rpc_server_thread>(
                    rpc_server_thread::config{
                        &mi
                    ,   &this->get_invoker()
                    ,   comm
                    ,   tag
                    }
                );
        }
    }
    
    virtual void register_handler(const rpc::untyped::register_handler_params& params) MGBASE_OVERRIDE
    {
        this->get_invoker().register_handler(params);
    }
    
private:
    
    mgbase::unique_ptr<server_thread_ptr []> ths_;
};

} // namespace mpi
} // namespace mgcom

