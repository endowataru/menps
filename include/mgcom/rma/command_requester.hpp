
#pragma once

#include <mgcom/rma/requester.hpp>

namespace mgcom {
namespace rma {

class command_requester
    : public requester
{
protected:
    command_requester() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~command_requester() /*noexcept*/ = default;
    
    
    
private:
    
};

} // namespace rma
} // namespace mgcom

