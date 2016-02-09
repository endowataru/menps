
#include "fjmpi_command_queue.impl.hpp"
#include "fjmpi_command_queue.hpp"
#include "common/rma/rma.hpp"
#include "device/fjmpi/rma/rma.hpp"

namespace mgcom {
namespace fjmpi {

namespace /*unnamed*/ {

fjmpi_command_queue g_queue;

} // unnamed namespace

void initialize_command_queue()
{
    g_queue.initialize();
}

void finalize_command_queue()
{
    g_queue.finalize();
}

void poll()
{
    // do nothing
}

} // namespace fjmpi

namespace mpi {

mpi_command_queue_base& g_queue = fjmpi::g_queue;

} // namespace mpi

namespace rma {
namespace untyped {

namespace /*unnamed*/ {

inline mgbase::uint64_t get_absolute_address(const local_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t laddr = addr.region.info;
    return laddr + addr.offset;
}

inline mgbase::uint64_t get_absolute_address(const remote_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t raddr = addr.region.info;
    return raddr + addr.offset;
}

} // unnamed namespace

bool try_remote_read_async_extra(
    const process_id_t          proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const int                   flags
,   const mgbase::operation&    on_complete
) {
    return fjmpi::g_queue.try_get(
        static_cast<int>(proc)
    ,   get_absolute_address(local_addr)
    ,   get_absolute_address(remote_addr)
    ,   size_in_bytes
    ,   flags
    ,   on_complete
    );
}

bool try_remote_read_async(
    const process_id_t          proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return try_remote_read_async_extra(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   fjmpi::g_queue.select_flags()
    ,   on_complete
    );
}

bool try_remote_write_async_extra(
    const process_id_t          proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const int                   flags
,   const mgbase::operation&    on_complete
) {
    return fjmpi::g_queue.try_put(
        static_cast<int>(proc)
    ,   get_absolute_address(local_addr)
    ,   get_absolute_address(remote_addr)
    ,   size_in_bytes
    ,   flags
    ,   on_complete
    );
}

bool try_remote_write_async(
    const process_id_t          proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return try_remote_write_async_extra(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   fjmpi::g_queue.select_flags()
    ,   on_complete
    );
}

} // namespace untyped
} // namespace rma

} // namespace mgcom

