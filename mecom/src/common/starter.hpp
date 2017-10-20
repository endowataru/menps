
#pragma once

#include <menps/mecom/common.hpp>

namespace menps {
namespace mecom {

class starter
{
protected:
    starter() noexcept = default;
    
public:
    virtual ~starter() = default;
    
    starter(const starter&) = delete;
    starter& operator = (const starter&) = delete;
    
    virtual endpoint& get_endpoint() = 0;
    
    virtual rma::requester& get_rma_requester() = 0;
    
    virtual rma::registrator& get_rma_registrator() = 0;
    
    virtual rma::allocator& get_rma_allocator() = 0;
    
    virtual rpc::requester& get_rpc_requester() = 0;
    
    virtual collective::requester& get_collective_requester() = 0;
};

} // namespace mecom
} // namespace menps

