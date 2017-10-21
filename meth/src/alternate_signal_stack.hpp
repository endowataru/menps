
#pragma once

#include <menps/meth/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meth {

class alternate_signal_stack
{
public:
    alternate_signal_stack(void* stack_first_ptr, mefdn::size_t size_in_bytes);
    
    ~alternate_signal_stack();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace meth
} // namespace menps

