
#pragma once

#include <mgbase/unique_ptr.hpp>

namespace mgult {
namespace sm {

class initializer
{
public:
    initializer();
    
    ~initializer();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace sm
} // namespace mgult

