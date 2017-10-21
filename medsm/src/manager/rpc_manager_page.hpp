
#pragma once

#include "rpc_manager_page_entry.hpp"
#include <vector>

namespace menps {
namespace medsm {

class rpc_manager_page
    : private rpc_manager_page_entry
{
public:
    class accessor;
    
    class proxy;
};

} // namespace medsm
} // namespace menps

