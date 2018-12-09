
#include "rma_comm.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include "requester.hpp"
#include "registrator.hpp"
#include "device/ibv/command/poll_thread.hpp"
#include "common/rma/default_allocator.hpp"

namespace menps {
namespace mecom {
namespace ibv {

namespace /*unnamed*/ {

class rma_comm_base
    : public rma_comm
{
protected:
    rma_comm_base(mecom::endpoint& ep, collective::requester& coll)
        : ep_(get_num_qps_per_proc())
    {
        ep_.collective_initialize(ep, coll);
        
        reg_ = make_rma_registrator(ep_);
        rma::registrator::set_instance(*reg_);
        
        alloc_ = rma::make_default_allocator(*reg_, 2ull << 30, 2ull << 30);
        rma::allocator::set_instance(*alloc_);
    }
    
    virtual ~rma_comm_base()
    {
        alloc_.reset();
        
        reg_.reset();
        
        ep_.finalize();
    }
    
    endpoint& get_endpoint() {
        return ep_;
    }
    
    bool is_reply_be()
    {
        const auto dev_attr = ep_.get_device().query_device();
        return is_only_masked_atomics(dev_attr);
    }
    
    static mefdn::size_t get_num_qps_per_proc() noexcept
    {
        if (const auto str = std::getenv("MECOM_IBV_NUM_QPS_PER_PROC"))
            return std::atoi(str);
        else
            return 1; // Default
    }
    
public:
    virtual rma::registrator& get_registrator() MEFDN_OVERRIDE {
        return *reg_;
    }
    
    virtual rma::allocator& get_allocator() MEFDN_OVERRIDE {
        return *alloc_;
    }
    
private:
    endpoint ep_;
    mefdn::unique_ptr<rma::registrator> reg_;
    mefdn::unique_ptr<rma::allocator> alloc_;
};

class direct_rma_comm
    : public rma_comm_base
{
public:
    direct_rma_comm(mecom::endpoint& ep, collective::requester& coll)
        : rma_comm_base(ep, coll)
    {
        req_ = make_rma_direct_requester({
            this->get_endpoint()
        ,   this->get_allocator() // depends on rma::registrator
        ,   ep
        ,   this->is_reply_be()
        ,   this->get_num_qps_per_proc()
        });
        
        rma::requester::set_instance(*req_);
    }
    
    ~direct_rma_comm() = default;
    
    virtual rma::requester& get_requester() MEFDN_OVERRIDE {
        return *req_;
    }
    
private:
    mefdn::unique_ptr<rma::requester> req_;
};

class scheduled_rma_comm
    : public rma_comm_base
{
public:
    scheduled_rma_comm(mecom::endpoint& ep, collective::requester& coll)
        : rma_comm_base(ep, coll)
    {
        //req_ = make_scheduled_rma_requester({
        req_ = make_rma_offload_requester({
            this->get_endpoint()
        ,   this->get_allocator() // depends on rma::registrator
        ,   ep
        ,   this->is_reply_be()
        ,   this->get_num_qps_per_proc()
        });
        
        rma::requester::set_instance(*req_);
    }
    
    ~scheduled_rma_comm() = default;
    
    virtual rma::requester& get_requester() MEFDN_OVERRIDE {
        return *req_;
    }
    
private:
    mefdn::unique_ptr<rma::requester> req_;
};

} // unnamed namespace

mefdn::unique_ptr<rma_comm> make_direct_rma_comm(mecom::endpoint& ep, collective::requester& coll)
{
    return mefdn::make_unique<direct_rma_comm>(ep, coll);
}

mefdn::unique_ptr<rma_comm> make_scheduled_rma_comm(mecom::endpoint& ep, collective::requester& coll)
{
    return mefdn::make_unique<scheduled_rma_comm>(ep, coll);
}

} // namespace ibv
} // namespace mecom
} // namespace menps

