
#pragma once

#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace medsm {

class sigsegv_catcher
{
public:
    struct config
    {
        mefdn::callback<bool (void*)>  on_signal;
        bool                            alter_stack;
    };
    
    explicit sigsegv_catcher(const config&);
    
    ~sigsegv_catcher();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm
} // namespace menps

