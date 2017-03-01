
#pragma once

#include "alltoall_queue_pairs.hpp"
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/device_list.hpp>
#include <mgdev/ibv/protection_domain.hpp>
#include <mgdev/ibv/device_context.hpp>
#include <mgdev/ibv/attributes.hpp>

namespace mgcom {
namespace ibv {

// TODO: conflicting names (mgcom::endpoint)

using namespace mgdev::ibv;

class endpoint
    : protected alltoall_queue_pairs
{
    static const int num_cqe = 1 << 18; // TODO
    
public:
    explicit endpoint(const index_t qp_count)
        : alltoall_queue_pairs(qp_count) { }
    
    void collective_initialize(mgcom::endpoint& ep, collective::requester& coll)
    {
        const auto devices = get_device_list();
        
        if (const char* const dev_name = get_device_name())
            ctx_ = open_device(devices.get_by_name(dev_name));
        else
            ctx_ = open_device(devices.get_by_index(0));
        
        cq_ = make_completion_queue(ctx_.get(), num_cqe);
        pd_ = make_protection_domain(ctx_.get());
        
        const auto port_num = this->get_port_number();
        
        alltoall_queue_pairs::create(ep, coll, *cq_.get(), *pd_.get(), port_num);
        
        auto dev_attr = ctx_.query_device();
        auto port_attr = ctx_.query_port(port_num);
        
        alltoall_queue_pairs::collective_start(dev_attr, port_attr, port_num);
    }
    
    void finalize()
    {
        alltoall_queue_pairs::destroy();
    }
    
    completion_queue& get_cq() { return cq_; }
    
    ibv_pd* get_pd() const MGBASE_NOEXCEPT {
        return pd_.get();
    }
    
private:
    static mgbase::uint8_t get_port_number() MGBASE_NOEXCEPT
    {
        if (const char* const port_str = std::getenv("MGCOM_IBV_PORT"))
            return static_cast<mgbase::uint8_t>(std::atoi(port_str));
        else
            return 1; // Default
    }
    
    static const char* get_device_name() MGBASE_NOEXCEPT
    {
        return std::getenv("MGCOM_IBV_DEVICE");
    }
    
public:
    using alltoall_queue_pairs::try_post_send;
    using alltoall_queue_pairs::get_qp_num_of_proc;
    using alltoall_queue_pairs::get_qp_count;
    
private:
    device_context      ctx_;
    completion_queue    cq_;
    protection_domain   pd_;
};

} // namespace ibv
} // namespace mgcom

