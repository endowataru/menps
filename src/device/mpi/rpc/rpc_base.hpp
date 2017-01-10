
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
        : comm_(
            mgdev::mpi::communicator::duplicate(
                mi
            ,   MPI_COMM_WORLD // TODO
            ,   "MGCOM_COMM_RPC"
            )
        )
    { }
    
protected:
    MPI_Comm get_comm() const MGBASE_NOEXCEPT
    {
        return comm_.get();
    }
    
    int get_server_tag() const MGBASE_NOEXCEPT
    {
        return 100; // TODO: adjustable
    }
    
    rpc::rpc_invoker& get_invoker() MGBASE_NOEXCEPT
    {
        return invoker_;
    }
    
private:
    rpc::rpc_invoker    invoker_;
    
    mgdev::mpi::communicator comm_;
};

} // namespace mpi
} // namespace mgcom

