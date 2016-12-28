
#pragma once

#include <mgbase/unique_ptr.hpp>

namespace mgth {

class alternate_signal_stack
{
public:
    alternate_signal_stack(void* stack_first_ptr, mgbase::size_t size_in_bytes);
    
    ~alternate_signal_stack();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgth

