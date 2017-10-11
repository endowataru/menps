
#pragma once

#include <mgbase/unique_ptr.hpp>
#include <mgbase/callback.hpp>

namespace mgdsm {

class sigsegv_catcher
{
public:
    struct config
    {
        mgbase::callback<bool (void*)>  on_signal;
        bool                            alter_stack;
    };
    
    explicit sigsegv_catcher(const config&);
    
    ~sigsegv_catcher();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgdsm

