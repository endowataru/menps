
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace rma {

void initialize();

void finalize();

/**
 * Low-level function of contiguous read.
 */
bool try_remote_read(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
);

/**
 * Low-level function of contiguous write.
 */
bool try_remote_write(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
);


/**
 * Low-level function of atomic read.
 */
bool try_remote_atomic_read_default(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   const local_address&    buf_addr
,   local_notifier          on_complete
);

/**
 * Low-level function of atomic write.
 */
bool try_remote_atomic_write_default(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   const local_address&    buf_addr
,   local_notifier          on_complete
);


/**
 * Low-level function of compare-and-swap.
 */
bool try_remote_compare_and_swap_default(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    expected_addr
,   const local_address&    desired_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);

/**
 * Low-level function of compare-and-swap.
 */
bool try_remote_fetch_and_add_default(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    value_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);

}

}


