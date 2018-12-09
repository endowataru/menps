
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

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {

namespace /*unnamed*/ {

class fjmpi_starter
    : public starter
{
public:
    fjmpi_starter(int* const argc, char*** const argv)
    {
        endpoint_ = fjmpi::make_endpoint(argc, argv);
        
        scheduler_ = fjmpi::make_scheduler(*endpoint_);
        
        rma_registrator_ = make_rma_registrator(scheduler_->get_fjmpi_interface());
        
        rma_allocator_ = rma::make_default_allocator(*rma_registrator_, 2ull << 30, 256ull << 20);
        
        rpc_requester_ = make_rpc_requester(scheduler_->get_fjmpi_interface(), scheduler_->get_mpi_interface(), *rma_allocator_, *rma_registrator_);
        
        rma_requester_ = fjmpi::make_rma_requester(scheduler_->get_command_producer(), *rpc_requester_);
        
        collective_requester_ = mpi::collective::make_requester(scheduler_->get_mpi_interface());
        
        scheduler_->get_mpi_interface().native_barrier({ MPI_COMM_WORLD });
        
        MEFDN_LOG_DEBUG("msg:Initialized.");
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
        
        MEFDN_LOG_DEBUG("msg:Finalized.");
    }
    
    virtual endpoint& get_endpoint() MEFDN_OVERRIDE {
        return *endpoint_;
    }
    virtual rma::requester& get_rma_requester() MEFDN_OVERRIDE {
        return *rma_requester_;
    }
    virtual rma::registrator& get_rma_registrator() MEFDN_OVERRIDE {
        return *rma_registrator_;
    }
    virtual rma::allocator& get_rma_allocator() MEFDN_OVERRIDE {
        return *rma_allocator_;
    }
    virtual rpc::requester& get_rpc_requester() MEFDN_OVERRIDE {
        return *rpc_requester_;
    }
    virtual collective::requester& get_collective_requester() MEFDN_OVERRIDE {
        return *collective_requester_;
    }
    
private:
    mefdn::unique_ptr<endpoint> endpoint_;
    mefdn::unique_ptr<scheduler> scheduler_;
    mefdn::unique_ptr<rma::registrator> rma_registrator_;
    mefdn::unique_ptr<rma::allocator> rma_allocator_;
    mefdn::unique_ptr<rma::requester> rma_requester_;
    mefdn::unique_ptr<rpc::requester> rpc_requester_;
    mefdn::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mefdn::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    return mefdn::make_unique<fjmpi_starter>(argc, argv);
}

} // namespace fjmpi
} // namespace mecom
} // namespace menps

