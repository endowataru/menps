
#pragma once

#include "alltoall_queue_pairs.hpp"
#include <menps/medev/ibv/completion_queue.hpp>
#include <menps/medev/ibv/device_list.hpp>
#include <menps/medev/ibv/protection_domain.hpp>
#include <menps/medev/ibv/device_context.hpp>
#include <menps/medev/ibv/attributes.hpp>

namespace menps {
namespace mecom {
namespace ibv {

// TODO: conflicting names (mecom::endpoint)

using namespace medev::ibv;

class endpoint
    : public alltoall_queue_pairs // XXX
{
public:
    explicit endpoint(const index_t qp_count)
        : alltoall_queue_pairs(qp_count) { }
    
    void collective_initialize(mecom::endpoint& ep, collective::requester& coll)
    {
        const auto devices = get_device_list();
        
        if (const char* const dev_name = get_device_name())
            ctx_ = open_device(devices.get_by_name(dev_name));
        else
            ctx_ = open_device(devices.get_by_index(0));
        
        pd_ = make_protection_domain(ctx_.get());
        
        const auto port_num = this->get_port_number();
        
        auto dev_attr = ctx_.query_device();
        auto port_attr = ctx_.query_port(port_num);
        
        alltoall_queue_pairs::collective_start(
            alltoall_queue_pairs::start_config{
                ep
            ,   coll
            ,   *ctx_
            ,   *pd_
            ,   dev_attr
            ,   port_attr
            ,   port_num
            }
        );
    }
    
    void finalize()
    {
        alltoall_queue_pairs::destroy();
    }
    
    device_context& get_device() { return ctx_; }
    
    ibv_pd* get_pd() const noexcept {
        return pd_.get();
    }
    
private:
    static mefdn::uint8_t get_port_number() noexcept
    {
        if (const char* const port_str = std::getenv("MECOM_IBV_PORT"))
            return static_cast<mefdn::uint8_t>(std::atoi(port_str));
        else
            return 1; // Default
    }
    
    static const char* get_device_name() noexcept
    {
        return std::getenv("MECOM_IBV_DEVICE");
    }
    
private:
    device_context      ctx_;
    protection_domain   pd_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

