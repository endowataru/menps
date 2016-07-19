
#include "rma_comm.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/completer.hpp"
#include "requester.hpp"
#include "registrator.hpp"
#include "device/ibv/command/poll_thread.hpp"
#include "common/rma/default_allocator.hpp"
#include "device/ibv/scheduler/scheduler.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class rma_comm_base
    : public rma_comm
{
protected:
    rma_comm_base()
    {
        ep_.collective_initialize();
        
        reg_ = make_rma_registrator(ep_);
        rma::registrator::set_instance(*reg_);
        
        alloc_ = rma::make_default_allocator(*reg_);
        rma::allocator::set_instance(*alloc_);
        
        comp_ = mgbase::make_unique<completer>(); // completer requires allocator
        
        poll_ = mgbase::make_unique<poll_thread>(ep_.get_cq(), *comp_);
    }
    
    virtual ~rma_comm_base()
    {
        poll_.reset();
        
        comp_.reset();
        
        alloc_.reset();
        
        reg_.reset();
        
        ep_.finalize();
    }
    
    endpoint& get_endpoint() {
        return ep_;
    }
    
    completer& get_completer() {
        return *comp_;
    }
    
public:
    virtual rma::registrator& get_registrator() MGBASE_OVERRIDE {
        return *reg_;
    }
    
    virtual rma::allocator& get_allocator() MGBASE_OVERRIDE {
        return *alloc_;
    }
    
private:
    endpoint ep_;
    mgbase::unique_ptr<completer> comp_;
    mgbase::unique_ptr<rma::registrator> reg_;
    mgbase::unique_ptr<rma::allocator> alloc_;
    mgbase::unique_ptr<poll_thread> poll_;
};

class direct_rma_comm
    : public rma_comm_base
{
public:
    direct_rma_comm()
    {
        req_ = make_rma_direct_requester(this->get_endpoint(), this->get_completer(), this->get_allocator()); // depends on rma::registrator
        rma::requester::set_instance(*req_);
    }
    
    virtual rma::requester& get_requester() MGBASE_OVERRIDE {
        return *req_;
    }
    
private:
    mgbase::unique_ptr<rma::requester> req_;
};

class scheduled_rma_comm
    : public rma_comm_base
{
public:
    scheduled_rma_comm()
    {
        req_ = make_scheduled_rma_requester(this->get_endpoint(), this->get_completer(), this->get_allocator()); // depends on rma::registrator
        rma::requester::set_instance(*req_);
    }
    
    virtual rma::requester& get_requester() MGBASE_OVERRIDE {
        return *req_;
    }
    
private:
    mgbase::unique_ptr<rma::requester> req_;
};

} // unnamed namespace

mgbase::unique_ptr<rma_comm> make_direct_rma_comm()
{
    return mgbase::make_unique<direct_rma_comm>();
}

mgbase::unique_ptr<rma_comm> make_scheduled_rma_comm()
{
    return mgbase::make_unique<scheduled_rma_comm>();
}

} // namespace ibv
} // namespace mgcom

