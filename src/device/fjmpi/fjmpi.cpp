
#include "device/fjmpi/command/fjmpi_command_queue.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "rma/rma.hpp"
#include "device/fjmpi/rpc/rpc.hpp"
#include "device/mpi/collective/collective.hpp"
#include "device/mpi/rma/atomic.hpp"
#include "common/rma/region_allocator.hpp"
#include "common/starter.hpp"

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
        endpoint_ = mpi::make_endpoint(argc, argv);
        
        rma_registrator_ = fjmpi::rma::make_registrator();
        rma::registrator::set_instance(*rma_registrator_);
        
        mgcom::fjmpi::initialize_command_queue();
        
        mgcom::rma::initialize_allocator();
        
        rpc_requester_ = fjmpi::rpc::make_requester();
        rpc::requester::set_instance(*rpc_requester_);
        
        rma_requester_ = fjmpi::rma::make_requester();
        rma::requester::set_instance(*rma_requester_);
        
        collective_requester_ = mpi::collective::make_requester();
        collective::requester::set_instance(*collective_requester_);
        
        mgcom::mpi::native_barrier();
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~fjmpi_starter()
    {
        collective::barrier();
        
        mgcom::mpi::native_barrier();
        
        collective_requester_.reset();
        
        rma_requester_.reset();
        
        rpc_requester_.reset();
        
        mgcom::rma::finalize_allocator();
        
        mgcom::fjmpi::finalize_command_queue();
        
        rma_registrator_.reset();
        
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
    return mgbase::unique_ptr<starter>(new fjmpi_starter(argc, argv));
}

} // namespace fjmpi
} // namespace mgcom

