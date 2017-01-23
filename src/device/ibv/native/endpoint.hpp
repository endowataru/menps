
#pragma once

#include "alltoall_queue_pairs.hpp"
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/device_list.hpp>
#include <mgdev/ibv/protection_domain.hpp>
#include <mgdev/ibv/context.hpp>
#include <mgdev/ibv/attributes.hpp>

namespace mgcom {
namespace ibv {

// TODO: conflicting names (mgcom::endpoint)

using namespace mgdev::ibv;

class endpoint
    : protected alltoall_queue_pairs
    , protected protection_domain
{
public:
    explicit endpoint(const index_t qp_count)
        : alltoall_queue_pairs(qp_count) { }
    
    void collective_initialize(mgcom::endpoint& ep, collective::requester& coll)
    {
        device_list devices;
        devices.get_list();
        
        if (const char* const dev_name = get_device_name())
            ctx_.open(devices.get_by_name(dev_name));
        else
            ctx_.open(devices.get_by_index(0));
        
        cq_.create(ctx_.get());
        protection_domain::alloc(ctx_.get());
        
        alltoall_queue_pairs::create(ep, coll, ctx_.get(), cq_.get(), protection_domain::get());
        
        device_attributes dev_attr;
        dev_attr.query(ctx_);
        
        port_attributes port_attr;
        port_attr.query(ctx_, get_port_number());
        
        alltoall_queue_pairs::collective_start(dev_attr, port_attr);
    }
    
    void finalize()
    {
        alltoall_queue_pairs::destroy();
        protection_domain::dealloc();
        cq_.destroy();
        ctx_.close();
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
    
    using protection_domain::register_memory;
    using protection_domain::deregister_memory;
    
    completion_queue& get_cq() { return cq_; }
    
private:
    context             ctx_;
    completion_queue    cq_;
};

} // namespace ibv
} // namespace mgcom

