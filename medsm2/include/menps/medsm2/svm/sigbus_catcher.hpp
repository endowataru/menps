
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

class sigbus_catcher
{
public:
    struct config
    {
        using on_signal_type = std::function<bool (void*, fault_kind_t)>;
        on_signal_type  on_signal;
        bool            alter_stack;
    };
    
    explicit sigbus_catcher(const config&);
    
    ~sigbus_catcher();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm2
} // namespace menps

