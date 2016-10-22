
#pragma once

#include "root_scheduler.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgult {

class scheduler_initializer
{
public:
    explicit scheduler_initializer(root_scheduler&);
    
    ~scheduler_initializer();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgult

