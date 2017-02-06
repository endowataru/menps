
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_interface.hpp"
#include <mgdev/mpi/communicator.hpp>
#include "rpc.hpp"
#include "common/rpc/rpc_invoker.impl.hpp"

namespace mgcom {
namespace mpi {

class rpc_base
    : public rpc::requester
{
public:
    explicit rpc_base(mpi_interface& mi)
        : mi_(mi)
        , comm_(
            mgdev::mpi::communicator::duplicate(
                mi
            ,   MPI_COMM_WORLD // TODO
            ,   "MGCOM_COMM_RPC"
            )
        )
    { }
    
protected:
    mpi_interface& get_mpi_interface() {
        return mi_;
    }
    
    MPI_Comm get_comm() const MGBASE_NOEXCEPT
    {
        return comm_.get();
    }
    
    int get_send_tag(const rpc::handler_id_t handler_id) const MGBASE_NOEXCEPT
    {
        return handler_id;
    }
    int get_recv_tag(const mgbase::size_t reply_id) const MGBASE_NOEXCEPT
    {
        return rpc::constants::max_num_handlers + reply_id;
    }
    
    rpc::rpc_invoker& get_invoker() MGBASE_NOEXCEPT
    {
        return invoker_;
    }
    
private:
    mpi_interface&      mi_;
    rpc::rpc_invoker    invoker_;
    
    mgdev::mpi::communicator comm_;
};

} // namespace mpi
} // namespace mgcom

