
#pragma once

#include <mgbase/unique_ptr.hpp>
#include <mgbase/callback.hpp>

namespace mgdsm {

class sigbus_catcher
{
public:
    struct config
    {
        mgbase::callback<bool (void*)>  on_signal;
        bool                            alter_stack;
    };
    
    explicit sigbus_catcher(const config&);
    
    ~sigbus_catcher();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgdsm


