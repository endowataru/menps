
#pragma once

#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {
namespace sm {

class initializer
{
public:
    initializer();
    
    ~initializer();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace sm
} // namespace meult
} // namespace menps

