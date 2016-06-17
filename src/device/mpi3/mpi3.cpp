
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "device/mpi/rpc/rpc.hpp"
#include "device/mpi3/rma/rma.hpp"
#include "device/mpi3/collective/collective.hpp"
#include "device/mpi3/command/mpi3_command_queue.hpp"
#include "common/rma/region_allocator.hpp"
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
        
        mpi3::initialize_command_queue();
        
        rma_registrator_ = mpi3::rma::make_registrator();
        rma::registrator::set_instance(*rma_registrator_);
        
        rma_requester_ = mpi3::rma::make_requester();
        rma::requester::set_instance(*rma_requester_);
        
        mgcom::rma::initialize_allocator();
        
        rpc_requester_ = mpi::rpc::make_requester();
        rpc::requester::set_instance(*rpc_requester_);
        
        collective_requester_ = mpi3::collective::make_requester();
        collective::requester::set_instance(*collective_requester_);
        
        mgcom::mpi::native_barrier();
        
        MGBASE_LOG_DEBUG("msg:Initialized.");
    }
    
    virtual ~mpi3_starter()
    {
        collective::barrier();
        
        mgcom::mpi::native_barrier();
        
        collective_requester_.reset();
        
        rpc_requester_.reset();
        
        mgcom::rma::finalize_allocator();
        
        rma_requester_.reset();
        
        rma_registrator_.reset();
        
        mpi3::finalize_command_queue();
        
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
    return mgbase::unique_ptr<starter>(new mpi3_starter(argc, argv));
}

} // namespace mpi3
} // namespace mgcom

