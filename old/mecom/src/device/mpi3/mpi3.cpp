
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi3/rma/rma.hpp"
#include "device/mpi3/collective/collective.hpp"
#include "device/mpi3/command/commander.hpp"
#include "common/rma/default_allocator.hpp"
#include <menps/mecom/collective.hpp>

#include <menps/mefdn/logger.hpp>

#include "common/starter.hpp"
#include "mpi3.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

namespace /*unnamed*/ {

class mpi3_starter
    : public starter
{
public:
    mpi3_starter(int* const argc, char*** const argv)
    {
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        commander_ = mpi3::make_commander(*endpoint_);
        
        rma_registrator_ = make_rma_registrator(commander_->get_mpi_interface(), commander_->get_win().get());
        
        rma_requester_ = make_rma_requester(commander_->get_mpi_interface(), commander_->get_win().get());
        
        rma_allocator_ = rma::make_default_allocator(*rma_registrator_, 2ull << 30, 2ull << 30);
        
        rpc_requester_ = mpi::make_rpc_requester(commander_->get_mpi_interface(), *endpoint_);
        
        collective_requester_ = make_collective_requester(commander_->get_mpi_interface());
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD /*TODO*/ });
        
        MEFDN_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~mpi3_starter()
    {
        collective_requester_->barrier();
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD /*TODO*/ });
        
        collective_requester_.reset();
        
        rpc_requester_.reset();
        
        rma_allocator_.reset();
        
        rma_requester_.reset();
        
        rma_registrator_.reset();
        
        commander_.reset();
        
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
    mefdn::unique_ptr<commander> commander_;
    mefdn::unique_ptr<rma::registrator> rma_registrator_;
    mefdn::unique_ptr<rma::allocator> rma_allocator_;
    mefdn::unique_ptr<rma::requester> rma_requester_;
    mefdn::unique_ptr<rpc::requester> rpc_requester_;
    mefdn::unique_ptr<collective::requester> collective_requester_;

};

} // unnamed namespace

mefdn::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mefdn::unique_ptr<starter>(new mpi3_starter(argc, argv));
}

} // namespace mpi3
} // namespace mecom
} // namespace menps

