
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace rma {

void initialize();

void finalize();

/**
 * Low-level function of contiguous write.
 */
bool try_write(
    const local_address&    local_addr
,   const remote_address&   remote_addr
,   index_t                 size_in_bytes
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

/**
 * Low-level function of contiguous read.
 */
bool try_read(
    const local_address&    local_addr
,   const remote_address&   remote_addr
,   index_t                 size_in_bytes
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

/**
 * Low level function of 64-bit atomic write.
 */
bool try_atomic_write_64(
    const local_address&    local_addr
,   const local_address&    buf_addr
,   const remote_address&   remote_addr
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

/**
 * Low level function of 64-bit atomic read.
 */
bool try_atomic_read_64(
    const local_address&    local_addr
,   const local_address&    buf_addr
,   const remote_address&   remote_addr
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

/**
 * Low-level function of 64-bit compare-and-swap.
 */
bool try_compare_and_swap_64(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* expected
,   const mgbase::uint64_t* desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

/**
 * Low-level function of 64-bit compare-and-swap.
 */
bool try_fetch_and_add_64(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* value
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

}

}


