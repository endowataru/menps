
#include <mgcom.hpp>
#include <mgbase/memory/aligned_alloc.hpp>

namespace mgcom {
namespace rma {
namespace untyped {

// TODO: replace naive implementation

registered_buffer allocate(index_t size_in_bytes) {
    void* const ptr = mgbase::aligned_alloc(buffer_alignment, size_in_bytes);
    const local_region region = register_region(ptr, size_in_bytes);
    
    registered_buffer buffer;
    buffer.addr = to_address(region);
    return buffer;
}

void deallocate(const registered_buffer& buf) {
    deregister_region(buf.addr.region);
    std::free(to_pointer(buf.addr)); // TODO : Is this correct?
}

} // namespace untyped
} // namespace rma
} // namespace mgcom

