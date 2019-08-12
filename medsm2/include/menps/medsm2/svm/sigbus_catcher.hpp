
#pragma once

#include <menps/mefdn/callback.hpp>

namespace menps {
namespace medsm2 {

class sigbus_catcher
{
public:
    struct config
    {
        mefdn::callback<bool (void*)>  on_signal;
        bool                            alter_stack;
    };
    
    explicit sigbus_catcher(const config&);
    
    ~sigbus_catcher();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm2
} // namespace menps

