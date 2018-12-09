
#pragma once

#include <menps/mecom/rma/requester.hpp>
#include <menps/mecom/rma/registrator.hpp>
#include <menps/mecom/rma/allocator.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class rma_comm
{
public:
    virtual ~rma_comm() = default;
    
    rma_comm(const rma_comm&) = delete;
    rma_comm& operator = (const rma_comm&) = delete;
    
    virtual rma::requester& get_requester() = 0;
    virtual rma::registrator& get_registrator() = 0;
    virtual rma::allocator& get_allocator() = 0;
    
protected:
    rma_comm() = default;
};

mefdn::unique_ptr<rma_comm> make_direct_rma_comm(mecom::endpoint&, collective::requester&);

mefdn::unique_ptr<rma_comm> make_scheduled_rma_comm(mecom::endpoint&, collective::requester&);

} // namespace ibv
} // namespace mecom
} // namespace menps

