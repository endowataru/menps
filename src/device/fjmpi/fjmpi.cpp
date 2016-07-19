
#include "device/mpi/mpi_base.hpp"
#include "rma/rma.hpp"
#include "device/mpi/collective/collective.hpp"
#include "device/mpi/rma/atomic.hpp"
#include "common/rma/default_allocator.hpp"
#include "starter.hpp"
#include "device/fjmpi/rpc/requester.hpp"
#include "device/fjmpi/rma/rma.hpp"
#include "endpoint.hpp"

#include "device/fjmpi/scheduler/scheduler.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace fjmpi {

namespace /*unnamed*/ {

class fjmpi_starter
    : public starter
{
public:
    fjmpi_starter(int* const argc, char*** const argv)
    {
        endpoint_ = fjmpi::make_endpoint(argc, argv);
        
        scheduler_ = fjmpi::make_scheduler();
        
        rma_registrator_ = make_rma_registrator(scheduler_->get_fjmpi_interface());
        rma::registrator::set_instance(*rma_registrator_);
        
        rma_allocator_ = rma::make_default_allocator(*rma_registrator_);
        rma::allocator::set_instance(*rma_allocator_);
        
        rpc_requester_ = make_rpc_requester(scheduler_->get_fjmpi_interface(), scheduler_->get_mpi_interface());
        rpc::requester::set_instance(*rpc_requester_);
        
        rma_requester_ = fjmpi::make_rma_requester(scheduler_->get_command_producer(), *rpc_requester_);
        rma::requester::set_instance(*rma_requester_);
        
        collective_requester_ = mpi::collective::make_requester(scheduler_->get_mpi_interface());
        collective::requester::set_instance(*collective_requester_);
        
        scheduler_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~fjmpi_starter()
    {
        scheduler_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        collective_requester_.reset();
        
        rma_requester_.reset();
        
        rpc_requester_.reset();
        
        rma_allocator_.reset();
        
        rma_registrator_.reset();
        
        scheduler_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<scheduler> scheduler_;
    mgbase::unique_ptr<rma::registrator> rma_registrator_;
    mgbase::unique_ptr<rma::allocator> rma_allocator_;
    mgbase::unique_ptr<rma::requester> rma_requester_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    return mgbase::make_unique<fjmpi_starter>(argc, argv);
}

} // namespace fjmpi
} // namespace mgcom

