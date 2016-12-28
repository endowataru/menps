
#include "alternate_signal_stack.hpp"

#include <signal.h>

#include <mgbase/logger.hpp>

namespace mgth {

class alternate_signal_stack_error
    : public std::exception { };

class alternate_signal_stack::impl
{
public:
    impl(void* const stack_first_ptr, const mgbase::size_t size_in_bytes)
    {
        // Note: The member order of stack_t on OS X
        //       is different from that on Linux.
        stack_t ss;
        ss.ss_sp = stack_first_ptr;
        ss.ss_size = size_in_bytes;
        ss.ss_flags = 0;
        
        const int ret = sigaltstack(&ss, &old_st_);
        
        if (ret == 0) {
            MGBASE_LOG_DEBUG(
                "msg:Called sigaltstack().\t"
                "stack_first_ptr:{:x}\t"
                "stack_size:{}\t"
                "ret:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(stack_first_ptr)
            ,   size_in_bytes
            ,   ret
            );
        }
        else {
            MGBASE_LOG_DEBUG(
                "msg:sigaltstack() failed.\t"
                "stack_first_ptr:{:x}\t"
                "stack_size:{}\t"
                "ret:{}\t"
                "errno:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(stack_first_ptr)
            ,   size_in_bytes
            ,   ret
            ,   errno
            );
            
            throw alternate_signal_stack_error{};
        }
    }
    
    ~impl()
    {
        // do nothing (TODO)
    }
    
private:
    stack_t old_st_;
};

alternate_signal_stack::alternate_signal_stack(void* const stack_first_ptr, const mgbase::size_t size_in_bytes)
    : impl_(new impl(stack_first_ptr, size_in_bytes))
    { }

alternate_signal_stack::~alternate_signal_stack() = default;

} // namespace mgth

