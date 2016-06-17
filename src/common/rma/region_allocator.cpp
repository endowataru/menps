
#include "region_allocator.ipp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

untyped::region_allocator g_alloc;

} // unnamed namespace

namespace untyped {

registered_buffer allocate(index_t size_in_bytes) {
    return g_alloc.allocate(size_in_bytes);
}

void deallocate(const registered_buffer& buf) {
    g_alloc.deallocate(buf);
}

} // namespace untyped

void initialize_allocator()
{
    g_alloc.initialize();
}
void finalize_allocator()
{
    g_alloc.finalize();
}

} // namespace rma
} // namespace mgcom

