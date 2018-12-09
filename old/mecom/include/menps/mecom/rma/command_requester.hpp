
#pragma once

#include <menps/mecom/rma/requester.hpp>

namespace menps {
namespace mecom {
namespace rma {

class command_requester
    : public requester
{
protected:
    command_requester() noexcept = default;
    
public:
    virtual ~command_requester() /*noexcept*/ = default;
    
    
    
private:
    
};

} // namespace rma
} // namespace mecom
} // namespace menps

