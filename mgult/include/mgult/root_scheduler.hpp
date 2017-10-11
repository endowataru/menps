
#pragma once

#include "scheduler.hpp"
#include <mgbase/functional/function.hpp>

namespace mgult {

typedef mgbase::function<void ()>   loop_func_type;

class root_scheduler
    : public scheduler
{
public:
    virtual void loop(const loop_func_type&) = 0;
};

} // namespace mgult

