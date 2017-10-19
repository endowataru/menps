
#pragma once

#include "scheduler.hpp"
#include <menps/mefdn/functional/function.hpp>

namespace menps {
namespace meult {

typedef mefdn::function<void ()>   loop_func_type;

class root_scheduler
    : public scheduler
{
public:
    virtual void loop(const loop_func_type&) = 0;
};

} // namespace meult
} // namespace menps

