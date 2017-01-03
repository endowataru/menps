
#include "mpi1.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi1/command/commander.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/rma/rma.hpp"
#include "device/mpi/collective/collective.hpp"
#include "common/rma/default_allocator.hpp"
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
        
        rma_registrator_ = mpi::make_rma_registrator();
        
        commander_ = mgcom::mpi1::make_commander();
        
        rpc_requester_ = mpi::make_rpc_requester(commander_->get_mpi_interface(), *endpoint_);
        
        rma_requester_ = mpi::make_rma_requester(*rpc_requester_, commander_->get_mpi_interface());
        
        rma_allocator_ = rma::make_default_allocator(*rma_registrator_, 2ull << 30, 2ull << 30);
        
        collective_requester_ = mpi::collective::make_requester(commander_->get_mpi_interface());
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~mpi1_starter()
    {
        collective_requester_->barrier();
        //collective::barrier();
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD });
        
        collective_requester_.reset();
        
        rma_allocator_.reset();
        
        rma_requester_.reset();
        
        rpc_requester_.reset();
        
        commander_.reset();
        
        rma_registrator_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
    virtual endpoint& get_endpoint() MGBASE_OVERRIDE {
        return *endpoint_;
    }
    virtual rma::requester& get_rma_requester() MGBASE_OVERRIDE {
        return *rma_requester_;
    }
    virtual rma::registrator& get_rma_registrator() MGBASE_OVERRIDE {
        return *rma_registrator_;
    }
    virtual rma::allocator& get_rma_allocator() MGBASE_OVERRIDE {
        return *rma_allocator_;
    }
    virtual rpc::requester& get_rpc_requester() MGBASE_OVERRIDE {
        return *rpc_requester_;
    }
    virtual collective::requester& get_collective_requester() MGBASE_OVERRIDE {
        return *collective_requester_;
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<commander> commander_;
    mgbase::unique_ptr<rma::registrator> rma_registrator_;
    mgbase::unique_ptr<rma::requester> rma_requester_;
    mgbase::unique_ptr<rma::allocator> rma_allocator_;
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

