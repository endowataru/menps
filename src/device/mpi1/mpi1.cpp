
#include "mpi1.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi1/command/commander.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/rma/rma.hpp"
#include "device/mpi/collective/collective.hpp"
#include "common/rma/region_allocator.hpp"
#include <mgcom/collective.hpp>

#include "common/starter.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace mpi1 {

namespace /*unnamed*/ {

class mpi1_starter
    : public starter
{
public:
    mpi1_starter(int* const argc, char*** const argv)
    {
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        rma_registrator_ = mpi::rma::make_registrator();
        rma::registrator::set_instance(*rma_registrator_);
        
        commander_ = mgcom::mpi1::make_commander();
        
        rpc_requester_ = mpi::rpc::make_requester(commander_->get_mpi_interface());
        rpc::requester::set_instance(*rpc_requester_);
        
        rma_requester_ = mpi::rma::make_requester(commander_->get_mpi_interface());
        rma::requester::set_instance(*rma_requester_);
        
        mgcom::rma::initialize_allocator();
        
        collective_requester_ = mpi::collective::make_requester(commander_->get_mpi_interface());
        collective::requester::set_instance(*collective_requester_);
        
        //collective_requester_->barrier();
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~mpi1_starter()
    {
        collective::barrier();
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        collective_requester_.reset();
        
        mgcom::rma::finalize_allocator();
        
        rma_requester_.reset();
        
        rpc_requester_.reset();
        
        commander_.reset();
        
        rma_registrator_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<commander> commander_;
    mgbase::unique_ptr<rma::registrator> rma_registrator_;
    mgbase::unique_ptr<rma::requester> rma_requester_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<starter>(new mpi1_starter(argc, argv));
}

} // namespace mpi1
} // namespace mgcom

