
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/collective/collective.hpp"
#include "ibv.hpp"
#include "common/starter.hpp"
#include "device/mpi1/command/commander.hpp"
#include "rma/rma_comm.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class ibv_starter
    : public starter
{
public:
    ibv_starter(int* const argc, char*** const argv)
    {
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        commander_ = mgcom::mpi1::make_commander();
        
        rpc_requester_ = mpi::rpc::make_requester(commander_->get_mpi_interface());
        rpc::requester::set_instance(*rpc_requester_);
        
        collective_requester_ = mpi::collective::make_requester(commander_->get_mpi_interface());
        collective::requester::set_instance(*collective_requester_);
        
        rma_ = make_direct_rma_comm();
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~ibv_starter()
    {
        collective::barrier();
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        rma_.reset();
        
        collective_requester_.reset();
        
        rpc_requester_.reset();
        
        commander_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<mpi1::commander> commander_;
    mgbase::unique_ptr<rma_comm> rma_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<starter>(new ibv_starter(argc, argv));
}

} // namespace ibv
} // namespace mgcom

