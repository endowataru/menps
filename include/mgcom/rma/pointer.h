
#pragma once

#include <mgcom/rma/address.h>

MGBASE_EXTERN_C_BEGIN

#ifdef MGBASE_CPLUSPLUS
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom::rma::local_pointer<type>
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom::rma::remote_pointer<type>
#else
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom_rma_local_address
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom_rma_remote_address
#endif

MGBASE_EXTERN_C_END

