
#pragma once

#include "root_scheduler.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {

class scheduler_initializer
{
public:
    explicit scheduler_initializer(root_scheduler&);
    
    ~scheduler_initializer();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace meult
} // namespace menps

