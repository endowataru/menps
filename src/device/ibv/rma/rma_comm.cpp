
#include "rma_comm.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include "requester.hpp"
#include "registrator.hpp"
#include "device/ibv/command/poll_thread.hpp"
#include "common/rma/default_allocator.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class rma_comm_base
    : public rma_comm
{
protected:
    rma_comm_base(mgcom::endpoint& ep, collective::requester& coll)
        : ep_(get_num_qps_per_proc())
    {
        ep_.collective_initialize(ep, coll);
        
        reg_ = make_rma_registrator(ep_);
        rma::registrator::set_instance(*reg_);
        
        alloc_ = rma::make_default_allocator(*reg_, 2ull << 30, 2ull << 30);
        rma::allocator::set_instance(*alloc_);
        
        #ifndef MGCOM_IBV_SEPARATE_CQ
        comp_sel_ = mgbase::make_unique<completion_selector>();
        
        poll_ =
            mgbase::make_unique<poll_thread>(
                ep_.get_cq()
            ,   *comp_sel_
            );
        #endif
    }
    
    virtual ~rma_comm_base()
    {
        #ifndef MGCOM_IBV_SEPARATE_CQ
        poll_.reset();
        
        comp_sel_.reset();
        #endif
        
        alloc_.reset();
        
        reg_.reset();
        
        ep_.finalize();
    }
    
    endpoint& get_endpoint() {
        return ep_;
    }
    
    #ifndef MGCOM_IBV_SEPARATE_CQ
    completion_selector& get_completion_selector() {
        return *comp_sel_;
    }
    #endif
    
    bool is_reply_be()
    {
        const auto dev_attr = ep_.get_device().query_device();
        return is_only_masked_atomics(dev_attr);
    }
    
    static mgbase::size_t get_num_qps_per_proc() MGBASE_NOEXCEPT
    {
        if (const auto str = std::getenv("MGCOM_IBV_NUM_QPS_PER_PROC"))
            return std::atoi(str);
        else
            return 1; // Default
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
    #ifndef MGCOM_IBV_SEPARATE_CQ
    mgbase::unique_ptr<completion_selector> comp_sel_;
    #endif
    mgbase::unique_ptr<rma::registrator> reg_;
    mgbase::unique_ptr<rma::allocator> alloc_;
    
    #ifndef MGCOM_IBV_SEPARATE_CQ
protected:
    mgbase::unique_ptr<poll_thread> poll_; // TODO: XXX
    #endif
};

class direct_rma_comm
    : public rma_comm_base
{
public:
    direct_rma_comm(mgcom::endpoint& ep, collective::requester& coll)
        : rma_comm_base(ep, coll)
    {
        req_ = make_rma_direct_requester({
            this->get_endpoint()
        #ifndef MGCOM_IBV_SEPARATE_CQ
        ,   this->get_completion_selector()
        #endif
        ,   this->get_allocator() // depends on rma::registrator
        ,   ep
        ,   this->is_reply_be()
        ,   this->get_num_qps_per_proc()
        });
        
        rma::requester::set_instance(*req_);
    }
    
    ~direct_rma_comm()
    {
        #ifndef MGCOM_IBV_SEPARATE_CQ
        // Destroy poll_thread first to stop accessing the completion callbacks.
        this->poll_.reset();
        #endif
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
    scheduled_rma_comm(mgcom::endpoint& ep, collective::requester& coll)
        : rma_comm_base(ep, coll)
    {
        req_ = make_scheduled_rma_requester({
            this->get_endpoint()
        #ifndef MGCOM_IBV_SEPARATE_CQ
        ,   this->get_completion_selector()
        #endif
        ,   this->get_allocator() // depends on rma::registrator
        ,   ep
        ,   this->is_reply_be()
        ,   this->get_num_qps_per_proc()
        });
        
        rma::requester::set_instance(*req_);
    }
    
    ~scheduled_rma_comm()
    {
        #ifndef MGCOM_IBV_SEPARATE_CQ
        // Destroy poll_thread first to stop accessing the completion callbacks.
        this->poll_.reset();
        #endif
    }
    
    virtual rma::requester& get_requester() MGBASE_OVERRIDE {
        return *req_;
    }
    
private:
    mgbase::unique_ptr<rma::requester> req_;
};

} // unnamed namespace

mgbase::unique_ptr<rma_comm> make_direct_rma_comm(mgcom::endpoint& ep, collective::requester& coll)
{
    return mgbase::make_unique<direct_rma_comm>(ep, coll);
}

mgbase::unique_ptr<rma_comm> make_scheduled_rma_comm(mgcom::endpoint& ep, collective::requester& coll)
{
    return mgbase::make_unique<scheduled_rma_comm>(ep, coll);
}

} // namespace ibv
} // namespace mgcom

