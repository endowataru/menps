
#pragma once

#include <menps/meult/generic/common.hpp>
#include "ult_id.hpp"

namespace menps {
namespace meult {

class scheduler
{
protected:
    scheduler() noexcept = default;
    
    scheduler(const scheduler&) noexcept = default;
    scheduler& operator = (const scheduler&) noexcept = default;
    
public:
    virtual ~scheduler() = default;
    
    struct allocated_ult {
        ult_id  id;
        void*   ptr;
    };
    
    virtual allocated_ult allocate(
        mefdn::size_t  alignment
    ,   mefdn::size_t  size
    ) = 0;
    
    virtual void fork(
        allocated_ult   th
    ,   fork_func_t     func
    ) = 0;
    
    virtual void join(ult_id) = 0;
    
    virtual void detach(ult_id) = 0;
    
    virtual void yield() = 0;
    
    MEFDN_NORETURN
    virtual void exit() = 0;
};

} // namespace meult
} // namespace menps

