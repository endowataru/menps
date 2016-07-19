
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi3/rma/rma.hpp"
#include "device/mpi3/collective/collective.hpp"
#include "device/mpi3/command/commander.hpp"
#include "common/rma/default_allocator.hpp"
#include <mgcom/collective.hpp>

#include <mgbase/logging/logger.hpp>

#include "common/starter.hpp"
#include "mpi3.hpp"

namespace mgcom {
namespace mpi3 {

namespace /*unnamed*/ {

class mpi3_starter
    : public starter
{
public:
    mpi3_starter(int* const argc, char*** const argv)
    {
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        commander_ = mpi3::make_commander();
        
        rma_registrator_ = make_rma_registrator(commander_->get_mpi_interface(), commander_->get_win());
        rma::registrator::set_instance(*rma_registrator_);
        
        rma_requester_ = make_rma_requester(commander_->get_mpi_interface(), commander_->get_win());
        rma::requester::set_instance(*rma_requester_);
        
        rma_allocator_ = rma::make_default_allocator(*rma_registrator_);
        rma::allocator::set_instance(*rma_allocator_);
        
        rpc_requester_ = mpi::rpc::make_requester(commander_->get_mpi_interface());
        rpc::requester::set_instance(*rpc_requester_);
        
        collective_requester_ = make_collective_requester(commander_->get_mpi_interface());
        collective::requester::set_instance(*collective_requester_);
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD /*TODO*/ });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~mpi3_starter()
    {
        collective::barrier();
        
        commander_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD /*TODO*/ });
        
        collective_requester_.reset();
        
        rpc_requester_.reset();
        
        rma_allocator_.reset();
        
        rma_requester_.reset();
        
        rma_registrator_.reset();
        
        commander_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<commander> commander_;
    mgbase::unique_ptr<rma::registrator> rma_registrator_;
    mgbase::unique_ptr<rma::allocator> rma_allocator_;
    mgbase::unique_ptr<rma::requester> rma_requester_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;

};

} // unnamed namespace


mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<starter>(new mpi3_starter(argc, argv));
}

} // namespace mpi3
} // namespace mgcom

