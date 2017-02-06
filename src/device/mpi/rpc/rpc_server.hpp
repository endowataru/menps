
#pragma once

#include "rpc_base.hpp"
#include "rpc_server_thread.hpp"

namespace mgcom {
namespace mpi {

class rpc_server
    : public virtual rpc_base
{
    #if 0
    static const index_t num_threads = 1;
    #endif
    
    typedef mgbase::unique_ptr<rpc_server_thread>   server_thread_ptr;
    
public:
    explicit rpc_server(mpi_interface& mi)
        : rpc_base(mi)
        , ths_(
            mgbase::make_unique<server_thread_ptr []>(rpc::constants::max_num_handlers)
        )
    {
        #if 0
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
        #endif
    }
    
    virtual void register_handler(const rpc::untyped::register_handler_params& params) MGBASE_OVERRIDE
    {
        this->get_invoker().register_handler(params);
        
        ths_[params.id] =
            mgbase::make_unique<rpc_server_thread>(
                rpc_server_thread::config{
                    &this->get_mpi_interface()
                ,   &this->get_invoker()
                ,   this->get_comm()
                ,   this->get_send_tag(params.id)
                }
            );
    }
    
private:
    mgbase::unique_ptr<server_thread_ptr []> ths_;
};

} // namespace mpi
} // namespace mgcom

