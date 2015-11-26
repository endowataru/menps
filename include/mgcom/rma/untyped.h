
#pragma once

#include <mgcom/common.h>

MGBASE_EXTERN_C_BEGIN

#define MGCOM_REGISTRATION_ALIGNMENT  4
#define MGCOM_BUFFER_ALIGNMENT        4

typedef uint64_t  mgcom_rma_address_offset_t;

/**
 * Region key.
 * Guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_region_key {
    void*    pointer;
    
    // (ibv)   info = rkey
    // (fjmpi) info = memid
    // (mpi3)  info = win_id
    uint64_t info;
}
mgcom_rma_region_key;

/**
 * Local registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_local_region {
    mgcom_rma_region_key key;
    
    // (ibv)   info = lkey
    // (fjmpi) info = laddr
    uint64_t info;
}
mgcom_rma_local_region;

/**
 * Remote registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_remote_region {
    mgcom_rma_region_key key;
    
    // (ibv)   info = (unused)
    // (fjmpi) info = raddr
    uint64_t info;
}
mgcom_rma_remote_region;

/**
 * Address of registered memory region located on the local process.
 */
typedef struct mgcom_rma_local_address {
    mgcom_rma_local_region            region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_rma_address_offset_t        offset;
}
mgcom_rma_local_address;

/**
 * Address of registered memory region located on a remote process.
 */
typedef struct mgcom_rma_remote_address {
    mgcom_rma_remote_region           region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_rma_address_offset_t        offset;
}
mgcom_rma_remote_address;


/**
 * Register a region located on the current process.
 */
mgcom_error_code_t mgcom_rma_register_region(
    void*                        local_pointer
,   mgcom_index_t                size_in_bytes
,   mgcom_rma_local_region*      result
) MGBASE_NOEXCEPT;


/**
 * Prepare a region located on a remote process.
 */
mgcom_error_code_t mgcom_rma_use_remote_region(
    mgcom_process_id_t           proc_id
,   mgcom_rma_region_key         key
,   mgcom_index_t                size_in_bytes
,   mgcom_rma_remote_region*     result
) MGBASE_NOEXCEPT;

/**
 * De-register the region located on the current process.
 */
mgcom_error_code_t mgcom_rma_deregister_region(
    mgcom_rma_local_region       region
,   void*                        local_pointer
,   mgcom_index_t                size_in_bytes
) MGBASE_NOEXCEPT;


/**
 * Registered buffer.
 */
typedef struct mgcom_rma_registered_buffer {
    mgcom_rma_local_address addr;
}
mgcom_rma_registered_buffer;

/**
 * Allocate a buffer from registered buffer pool.
 */
mgcom_error_code_t mgcom_rma_allocate(
    mgcom_index_t                   size_in_bytes
,   mgcom_rma_registered_buffer*    result
) MGBASE_NOEXCEPT;

/**
 * Deallocate a buffer allocated from registered buffer pool.
 */
mgcom_error_code_t mgcom_rma_deallocate(
    mgcom_rma_registered_buffer     buffer
) MGBASE_NOEXCEPT;


/// Control block for non-blocking contiguous read.
typedef struct mgcom_rma_remote_read_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
}
mgcom_rma_remote_read_cb;

/// Control block for non-blocking contiguous write.
typedef struct mgcom_rma_remote_write_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
}
mgcom_rma_remote_write_cb;



/// Control block for non-blocking strided write.
typedef struct mgcom_rma_write_strided_cb {
    mgbase_control_cb_common    common;
}
mgcom_rma_write_strided_cb;

/**
 * Non-blockng strided write.
 */
mgcom_error_code_t mgcom_rma_write_strided_nb(
    mgcom_rma_write_strided_cb* cb
,   mgcom_rma_local_address     local_addr
,   mgcom_index_t*              local_stride
,   mgcom_rma_remote_address    remote_addr
,   mgcom_index_t*              remote_stride
,   mgcom_index_t*              count
,   mgcom_index_t               stride_level
,   mgcom_process_id_t          dest_proc
) MGBASE_NOEXCEPT;


/// Control block for non-blocking strided read.
typedef struct mgcom_rma_read_strided_cb {
    mgbase_control_cb_common    common;
}
mgcom_rma_read_strided_cb;

/**
 * Non-blockng strided read.
 */
mgcom_error_code_t mgcom_rma_read_strided_nb(
    mgcom_rma_read_strided_cb*  cb
,   mgcom_rma_local_address     local_addr
,   mgcom_index_t*              local_stride
,   mgcom_rma_remote_address    remote_addr
,   mgcom_index_t*              remote_stride
,   mgcom_index_t*              count
,   mgcom_index_t               stride_level
,   mgcom_process_id_t          dest_proc
) MGBASE_NOEXCEPT;


/**
 * Default integer type for atomic operations.
 */
typedef mgbase_uint64_t     mgcom_rma_atomic_default_t;


/// Control block for non-blocking remote atomic read.
typedef struct mgcom_rma_atomic_read_default_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
}
mgcom_rma_atomic_read_default_cb;

/// Control block for non-blocking remote atomic write.
typedef struct mgcom_rma_atomic_write_default_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
}
mgcom_rma_atomic_write_default_cb;


/// Control block for non-blocking local compare-and-swap.
typedef struct mgcom_rma_local_compare_and_swap_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_local_compare_and_swap_default_cb;

/// Control block for non-blocking local fetch-and-add.
typedef struct mgcom_rma_local_fetch_and_add_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     diff_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_local_fetch_and_add_default_cb;


/// Control block for non-blocking remote compare-and-swap.
typedef struct mgcom_rma_remote_compare_and_swap_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_remote_compare_and_swap_default_cb;

/// Control block for non-blocking fetch-and-add.
typedef struct mgcom_rma_remote_fetch_and_add_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     value_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_remote_fetch_and__default_cb;


MGBASE_EXTERN_C_END

