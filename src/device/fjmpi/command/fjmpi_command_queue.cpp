
#include "fjmpi_command_queue.impl.hpp"
#include "fjmpi_command_queue.hpp"
#include "common/rma/rma.hpp"
#include "device/fjmpi/rma/rma.hpp"
#include "device/mpi/rma/rma.hpp"

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

} // namespace fjmpi

namespace mpi {

fjmpi::fjmpi_command_queue& g_queue = fjmpi::g_queue;

} // namespace mpi

namespace fjmpi {

namespace rma {
namespace untyped {

inline mgbase::uint64_t get_absolute_address(const local_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t laddr = addr.region.info;
    return laddr + addr.offset;
}

inline mgbase::uint64_t get_absolute_address(const remote_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t raddr = addr.region.info;
    return raddr + addr.offset;
}

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

bool try_read_async(const untyped::read_params& params)
{
    return try_remote_read_async(
        params.src_proc
    ,   params.src_raddr
    ,   params.dest_laddr
    ,   params.size_in_bytes
    ,   params.on_complete
    );
}

bool try_write_async(const untyped::write_params& params)
{
    return try_remote_write_async(
        params.dest_proc
    ,   params.dest_raddr
    ,   params.src_laddr
    ,   params.size_in_bytes
    ,   params.on_complete
    );
}

} // namespace untyped
} // namespace rma

} // namespace fjmpi
} // namespace mgcom

#include "device/mpi/command/mpi_interface.impl.hpp"

