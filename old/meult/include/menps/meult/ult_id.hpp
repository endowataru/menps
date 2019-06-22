
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace meult {

typedef mefdn::uint32_t    ult_local_id_t;

union ult_id
{
    void* ptr;
    
    struct distributed
    {
        mefdn::uint32_t    proc; // TODO: mgcom::process_id_t
        ult_local_id_t      local_id;
    }
    di;
};

MEFDN_STATIC_ASSERT(mefdn::is_trivially_copyable<ult_id>::value);

//MEFDN_CONSTEXPR
inline ult_id make_invalid_ult_id() noexcept
{
    return { reinterpret_cast<void*>(-1) };
}

//MEFDN_CONSTEXPR
inline bool is_invalid_ult_id(const ult_id& id) noexcept
{
    return id.ptr == make_invalid_ult_id().ptr;
}

} // namespace meult
} // namespace menps

