
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rma/registrator.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class rma_comm
{
public:
    virtual ~rma_comm() = default;
    
    rma_comm(const rma_comm&) = delete;
    rma_comm& operator = (const rma_comm&) = delete;
    
    virtual rma::requester& get_requester() = 0;
    virtual rma::registrator& get_registrator() = 0;
    
protected:
    rma_comm() = default;
};

mgbase::unique_ptr<rma_comm> make_direct_rma_comm();

} // namespace ibv
} // namespace mgcom

