
#pragma once

#include "rpc_base.hpp"
#include "rpc_server_thread.hpp"

namespace menps {
namespace mecom {
namespace mpi {

class rpc_server
    : public virtual rpc_base
{
    typedef mefdn::unique_ptr<rpc_server_thread>   server_thread_ptr;
    
public:
    explicit rpc_server(mpi_interface& mi)
        : rpc_base(mi)
        , ths_(
            mefdn::make_unique<server_thread_ptr []>(rpc::constants::max_num_handlers)
        )
    {
    }
    
    virtual void add_handler(const rpc::untyped::add_handler_params& params) MEFDN_OVERRIDE
    {
        this->get_invoker().add_handler(params);
        
        ths_[params.id] =
            mefdn::make_unique<rpc_server_thread>(
                rpc_server_thread::config{
                    &this->get_mpi_interface()
                ,   &this->get_invoker()
                ,   this->get_comm()
                ,   this->get_send_tag(params.id)
                }
            );
    }
    
private:
    mefdn::unique_ptr<server_thread_ptr []> ths_;
};

} // namespace mpi
} // namespace mecom
} // namespace menps

