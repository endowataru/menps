
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
 * Low-level function of remote compare-and-swap.
 */
bool try_remote_compare_and_swap_default(
    process_id_t            target_proc
,   const remote_address&   target_addr
,   const local_address&    expected_addr
,   const local_address&    desired_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);

/**
 * Low-level function of remote fetch-and-add.
 */
bool try_remote_fetch_and_add_default(
    process_id_t            target_proc
,   const remote_address&   target_addr
,   const local_address&    value_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);


/**
 * Low-level function of local compare-and-swap.
 */
bool try_local_compare_and_swap_default(
    const local_address&    target_addr
,   const local_address&    expected_addr
,   const local_address&    desired_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);

/**
 * Low-level function of local fetch-and-add.
 */
bool try_local_fetch_and_add_default(
    const local_address&    target_addr
,   const local_address&    value_addr
,   const local_address&    result_addr
,   local_notifier          on_complete
);

}

}


