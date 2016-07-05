
#include "rma_comm.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/completer.hpp"
#include "requester.hpp"
#include "registrator.hpp"
#include "device/ibv/command/poll_thread.hpp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class direct_rma_comm
    : public rma_comm
{
public:
    direct_rma_comm()
    {
        ep_.collective_initialize();
        
        reg_ = make_rma_registrator(ep_);
        rma::registrator::set_instance(*reg_);
        
        rma::initialize_allocator();
        
        comp_ = mgbase::make_unique<completer>(); // completer requires allocator
        
        req_ = make_rma_direct_requester(ep_, *comp_); // depends on rma::registrator
        rma::requester::set_instance(*req_);
        
        poll_ = mgbase::make_unique<poll_thread>(ep_.get_cq(), *comp_);
    }
    
    virtual ~direct_rma_comm()
    {
        poll_.reset();
        
        req_.reset();
        
        comp_.reset();
         
        rma::finalize_allocator();
        
        reg_.reset();
        
        ep_.finalize();
    }
    
    virtual rma::requester& get_requester() MGBASE_OVERRIDE {
        return *req_;
    }
    virtual rma::registrator& get_registrator() MGBASE_OVERRIDE {
        return *reg_;
    }
    
private:
    endpoint ep_;
    mgbase::unique_ptr<completer> comp_;
    mgbase::unique_ptr<rma::requester> req_;
    mgbase::unique_ptr<rma::registrator> reg_;
    mgbase::unique_ptr<poll_thread> poll_;
};

} // unnamed namespace

mgbase::unique_ptr<rma_comm> make_direct_rma_comm()
{
    return mgbase::make_unique<direct_rma_comm>();
}

} // namespace ibv
} // namespace mgcom

