
#pragma once

#include <menps/mefdn/lang.hpp>

MEFDN_EXTERN_C_BEGIN

#define MECOM_REGISTRATION_ALIGNMENT  4
#define MECOM_BUFFER_ALIGNMENT        4

typedef uint64_t  mecom_rma_address_offset_t;

/**
 * Region key.
 * Guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mecom_rma_region_key {
    void*    pointer;
    
    // (ibv)   info = rkey
    // (fjmpi) info = memid
    // (mpi3)  info = win_id
    uint64_t info;
}
mecom_rma_region_key;

/**
 * Local registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mecom_rma_local_region {
    mecom_rma_region_key key;
    
    // (ibv)   info = mr
    // (fjmpi) info = laddr
    uint64_t info;
}
mecom_rma_local_region;

/**
 * Remote registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mecom_rma_remote_region {
    mecom_rma_region_key key;
    
    // (ibv)   info = (unused)
    // (fjmpi) info = raddr
    uint64_t info;
}
mecom_rma_remote_region;

/**
 * Address of registered memory region located on the local process.
 */
typedef struct mecom_rma_local_address {
    mecom_rma_local_region            region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MECOM_REGISTRATION_ALIGNMENT.
     */
    mecom_rma_address_offset_t        offset;
}
mecom_rma_local_address;

/**
 * Address of registered memory region located on a remote process.
 */
typedef struct mecom_rma_remote_address {
    mecom_rma_remote_region           region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MECOM_REGISTRATION_ALIGNMENT.
     */
    mecom_rma_address_offset_t        offset;
}
mecom_rma_remote_address;

/**
 * Registered buffer.
 */
typedef struct mecom_rma_registered_buffer {
    mecom_rma_local_address addr;
}
mecom_rma_registered_buffer;

MEFDN_EXTERN_C_END

