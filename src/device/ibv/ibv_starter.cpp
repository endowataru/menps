
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/collective/collective.hpp"
#include "ibv_starter.hpp"
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
        
        rpc_requester_ = mpi::make_rpc_requester(commander_->get_mpi_interface(), *endpoint_);
        
        collective_requester_ = mpi::collective::make_requester(commander_->get_mpi_interface());
        
        if (is_scheduled())
            rma_ = make_scheduled_rma_comm(*endpoint_, *collective_requester_);
        else
            rma_ = make_direct_rma_comm(*endpoint_, *collective_requester_);
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD });
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~ibv_starter()
    {
        collective::barrier();
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD });
        
        rma_.reset();
        
        collective_requester_.reset();
        
        rpc_requester_.reset();
        
        commander_.reset();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
    virtual endpoint& get_endpoint() MGBASE_OVERRIDE {
        return *endpoint_;
    }
    virtual rma::requester& get_rma_requester() MGBASE_OVERRIDE {
        return rma_->get_requester();
    }
    virtual rma::registrator& get_rma_registrator() MGBASE_OVERRIDE {
        return rma_->get_registrator();
    }
    virtual rma::allocator& get_rma_allocator() MGBASE_OVERRIDE {
        return rma_->get_allocator();
    }
    virtual rpc::requester& get_rpc_requester() MGBASE_OVERRIDE {
        return *rpc_requester_;
    }
    virtual collective::requester& get_collective_requester() MGBASE_OVERRIDE {
        return *collective_requester_;
    }
    
private:
    static bool is_scheduled() MGBASE_NOEXCEPT
    {
        if (const char* const direct = std::getenv("MGCOM_IBV_DIRECT"))
            return std::atoi(direct) == 0;
        else
            return true; // Default
    }
    
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<mpi1::commander> commander_;
    mgbase::unique_ptr<rma_comm> rma_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    return mgbase::make_unique<ibv_starter>(argc, argv);
}

} // namespace ibv
} // namespace mgcom

