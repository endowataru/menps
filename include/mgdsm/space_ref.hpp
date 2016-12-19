
#pragma once

#include <mgdsm/space.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class space_ref
{
public:
    explicit space_ref(space* const sp)
        : sp_(sp)
    { }
    
private:
    mgbase::unique_ptr<space> sp_;
};

} // namespace mgdsm

