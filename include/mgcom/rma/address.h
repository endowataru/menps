
#pragma once

#include <mgcom/rma/common.h>

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
    
    // (ibv)   info = mr
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
 * Registered buffer.
 */
typedef struct mgcom_rma_registered_buffer {
    mgcom_rma_local_address addr;
}
mgcom_rma_registered_buffer;

MGBASE_EXTERN_C_END

