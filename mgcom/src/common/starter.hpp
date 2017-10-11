
#pragma once

#include <mgcom/common.hpp>

namespace mgcom {

class starter
    : mgbase::noncopyable
{
public:
    virtual ~starter() MGBASE_EMPTY_DEFINITION
    
    virtual endpoint& get_endpoint() = 0;
    
    virtual rma::requester& get_rma_requester() = 0;
    
    virtual rma::registrator& get_rma_registrator() = 0;
    
    virtual rma::allocator& get_rma_allocator() = 0;
    
    virtual rpc::requester& get_rpc_requester() = 0;
    
    virtual collective::requester& get_collective_requester() = 0;
};

} // namespace mgcom

