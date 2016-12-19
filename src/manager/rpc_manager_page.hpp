
#pragma once

#include "rpc_manager_page_entry.hpp"
#include <vector>

namespace mgdsm {

class rpc_manager_page
    : private rpc_manager_page_entry
{
public:
    class accessor;
    
    class proxy;
};

} // namespace mgdsm

