
#pragma once

#include <mgult/ult_id.hpp>

namespace mgult {

struct ult_id_worker_traits_base
{
    typedef ult_id          ult_id_type;
    
    // TODO: simpler solution to eliminate template here
    template <typename Ref>
    static void check_ult_id(Ref& th, const ult_id id)
    {
        MGBASE_ASSERT(th.is_valid());
        MGBASE_ASSERT(th.get_id().ptr == id.ptr);
    }
};

} // namespace mgult


