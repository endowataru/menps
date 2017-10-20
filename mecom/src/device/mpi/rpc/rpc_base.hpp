
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_interface.hpp"
#include <menps/medev/mpi/communicator.hpp>
#include "rpc.hpp"
#include "common/rpc/rpc_invoker.impl.hpp"

namespace menps {
namespace mecom {
namespace mpi {

class rpc_base
    : public rpc::requester
{
public:
    explicit rpc_base(mpi_interface& mi)
        : mi_(mi)
        , comm_(
            medev::mpi::communicator::duplicate(
                mi
            ,   MPI_COMM_WORLD // TODO
            ,   "MECOM_COMM_RPC"
            )
        )
    { }
    
protected:
    mpi_interface& get_mpi_interface() {
        return mi_;
    }
    
    MPI_Comm get_comm() const noexcept
    {
        return comm_.get();
    }
    
    int get_send_tag(const rpc::handler_id_t handler_id) const noexcept
    {
        return handler_id;
    }
    int get_recv_tag(const mefdn::size_t reply_id) const noexcept
    {
        return rpc::constants::max_num_handlers + reply_id;
    }
    
    rpc::rpc_invoker& get_invoker() noexcept
    {
        return invoker_;
    }
    
private:
    mpi_interface&      mi_;
    rpc::rpc_invoker    invoker_;
    
    medev::mpi::communicator comm_;
};

} // namespace mpi
} // namespace mecom
} // namespace menps

