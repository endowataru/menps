
#pragma once

#include <mgult/generic/common.hpp>
#include "ult_id.hpp"

namespace mgult {

class scheduler
{
protected:
    scheduler() MGBASE_DEFAULT_NOEXCEPT = default;
    
    scheduler(const scheduler&) MGBASE_DEFAULT_NOEXCEPT = default;
    scheduler& operator = (const scheduler&) MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~scheduler() = default;
    
    struct allocated_ult {
        ult_id  id;
        void*   ptr;
    };
    
    virtual allocated_ult allocate(
        mgbase::size_t  alignment
    ,   mgbase::size_t  size
    ) = 0;
    
    virtual void fork(
        allocated_ult   th
    ,   fork_func_t     func
    ) = 0;
    
    virtual void join(ult_id) = 0;
    
    virtual void detach(ult_id) = 0;
    
    virtual void yield() = 0;
    
    MGBASE_NORETURN
    virtual void exit() = 0;
};

} // namespace mgult

