
#pragma once

#include <menps/mefdn/callback.hpp>
#include <menps/mecom/common.hpp>

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom {

class delegator
{
protected:
    delegator() noexcept = default;

public:
    virtual ~delegator() = default;
    
    delegator(const delegator&) = delete;
    delegator& operator = (const delegator&) = delete;
    
    struct execute_params
    {
        mefdn::callback<void (void*)> setter;
        bool (*delegated) (const void*);
    };
    
    MEFDN_NODISCARD
    virtual bool try_execute(const execute_params&) = 0;
    
    virtual mefdn::size_t get_buffer_size() = 0;
};

} // namespace mecom
} // namespace menps

