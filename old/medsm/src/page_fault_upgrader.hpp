
#pragma once

#include "protector/protector_space.hpp"
//#include "access_history.hpp"

namespace menps {
namespace medsm {

class page_fault_upgrader
{
public:
    struct config {
        protector_space&    sp;
        access_history&     hist;
    };
    
    explicit page_fault_upgrader(const config&);
    
    ~page_fault_upgrader();
    
    void enable_on_this_thread();
    
    void disable_on_this_thread();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm
} // namespace menps

