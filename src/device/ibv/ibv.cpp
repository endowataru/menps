
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "device/mpi1/command/mpi1_command_queue.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "common/rma/region_allocator.hpp"
#include "device/mpi/collective/collective.hpp"
#include "ibv.hpp"
#include "rma/rma.hpp"
#include "common/starter.hpp"
#include <mgcom/collective.hpp>

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
        
        mgcom::mpi1::initialize_command_queue();
        
        rma_registrator_ = ibv::rma::make_registrator();
        rma::registrator::set_instance(*rma_registrator_);
        
        rpc_requester_ = mpi::rpc::make_requester();
        rpc::requester::set_instance(*rpc_requester_);
        
        rma_requester_ = ibv::rma::make_requester();
        rma::requester::set_instance(*rma_requester_);
        
        collective_requester_ = mpi::collective::make_requester();
        collective::requester::set_instance(*collective_requester_);
        
        mgcom::ibv::initialize();
        
        mgcom::mpi::native_barrier();
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~ibv_starter()
    {
        collective::barrier();
        
        mgcom::mpi::native_barrier();
        
        mgcom::ibv::finalize();
        
        collective_requester_.reset();
        
        rma_requester_.reset();
        
        rpc_requester_.reset();
        
        rma_registrator_.reset();
        
        mgcom::mpi1::finalize_command_queue();
        
        endpoint_.reset();
        
        MGBASE_LOG_DEBUG("msg:Finalized.");
    }
    
private:
    mgbase::unique_ptr<endpoint> endpoint_;
    mgbase::unique_ptr<rma::registrator> rma_registrator_;
    mgbase::unique_ptr<rma::requester> rma_requester_;
    mgbase::unique_ptr<rpc::requester> rpc_requester_;
    mgbase::unique_ptr<collective::requester> collective_requester_;
};

} // unnamed namespace

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<starter>(new ibv_starter(argc, argv));
}
#if 0

void initialize(int* argc, char*** argv)
{
    mgcom::mpi::initialize(argc, argv);
    
    mgcom::mpi1::initialize_command_queue();
    
    rpc::initialize();
    
    collective::initialize();
    
    ibv::initialize();
    
    mgcom::mpi::native_barrier();
    
    MGBASE_LOG_DEBUG("msg:Initialized.");
}

void finalize()
{
    collective::barrier();
    
    mgcom::mpi::native_barrier();
    
    ibv::finalize();
    
    collective::finalize();
    
    rpc::finalize();
    
    mgcom::mpi1::finalize_command_queue();
    
    mgcom::mpi::finalize();
    
    MGBASE_LOG_DEBUG("msg:Finalized.");
}

#endif

} // namespace ibv
} // namespace mgcom

