
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi/collective/collective.hpp"
#include "ibv_starter.hpp"
#include "common/starter.hpp"
#include "device/mpi1/command/commander.hpp"
#include "rma/rma_comm.hpp"

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {
namespace ibv {

namespace /*unnamed*/ {

class ibv_starter
    : public starter
{
public:
    /*implicit*/ ibv_starter(int* const argc, char*** const argv)
    {
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        commander_ = mecom::mpi1::make_commander();
        
        rpc_requester_ = mpi::make_rpc_requester(commander_->get_mpi_interface(), *endpoint_);
        
        collective_requester_ = mpi::collective::make_requester(commander_->get_mpi_interface());
        
        if (is_scheduled())
            rma_ = make_scheduled_rma_comm(*endpoint_, *collective_requester_);
        else
            rma_ = make_direct_rma_comm(*endpoint_, *collective_requester_);
        
        commander_->get_mpi_interface().barrier({ MPI_COMM_WORLD });
        
        MEFDN_LOG_DEBUG("msg:Initialized.");
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
        
        MEFDN_LOG_DEBUG("msg:Finalized.");
    }
    
    virtual endpoint& get_endpoint() MEFDN_OVERRIDE {
        return *endpoint_;
    }
    virtual rma::requester& get_rma_requester() MEFDN_OVERRIDE {
        return rma_->get_requester();
    }
    virtual rma::registrator& get_rma_registrator() MEFDN_OVERRIDE {
        return rma_->get_registrator();
    }
    virtual rma::allocator& get_rma_allocator() MEFDN_OVERRIDE {
        return rma_->get_allocator();
    }
    virtual rpc::requester& get_rpc_requester() MEFDN_OVERRIDE {
        return *rpc_requester_;
    }
    virtual collective::requester& get_collective_requester() MEFDN_OVERRIDE {
        return *collective_requester_;
    }
    
private:
    static bool is_scheduled() noexcept
    {
        if (const char* const direct = std::getenv("MECOM_IBV_DIRECT"))
            return std::atoi(direct) == 0;
        else
            return true; // Default
    }
    
    mefdn::unique_ptr<endpoint> endpoint_;
    mefdn::unique_ptr<mpi1::commander> commander_;
    mefdn::unique_ptr<rma_comm> rma_;
    mefdn::unique_ptr<rpc::requester> rpc_requester_;
    mefdn::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mefdn::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    return mefdn::make_unique<ibv_starter>(argc, argv);
}

} // namespace ibv
} // namespace mecom
} // namespace menps

