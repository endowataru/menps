
#include <menps/medev/ucx/uct/allocated_memory.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void allocated_memory_deleter::operator () (uct_allocated_memory_t&& mem) const noexcept
{
    uct_iface_mem_free(&mem);
}

allocated_memory allocate_memory(
    uct_iface_t* const  iface
,   const size_t        length
,   const unsigned int  flags
,   const char* const   name
) {
    uct_allocated_memory mem = uct_allocated_memory();
    
    const auto ret = uct_iface_mem_alloc(iface, length, flags, name, &mem);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_mem_alloc() failed", ret);
    }
    
    return allocated_memory(mem);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

