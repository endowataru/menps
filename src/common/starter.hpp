
#pragma once

#include <mgcom/common.hpp>

namespace mgcom {

namespace rma {

class requester;

} // namespace rma

namespace rpc {

class requester;

} // namespace rpc

namespace collective {

class requester;

} // namespace collective



class starter
    : mgbase::noncopyable
{
public:
    virtual ~starter() MGBASE_EMPTY_DEFINITION
    
    /*
    virtual endpoint& get_endpoint() = 0;
    
    virtual rma::requester& get_rma() = 0;
    
    virtual rpc::requester& get_rpc() = 0;
    
    virtual collective::requester& get_collective() = 0;*/
    
};

} // namespace mgcom

