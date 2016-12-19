
#pragma once

#include "app_space.hpp"
#include "app_space_indexer.hpp"

namespace mgdsm {

class page_fault_upgrader
{
public:
    struct config {
        app_space&          space;
        app_space_indexer&  indexer;
    };
    
    explicit page_fault_upgrader(const config&);
    
    ~page_fault_upgrader();
    
    void enable_on_this_thread();
    
    void disable_on_this_thread();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgdsm

