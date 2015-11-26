
#pragma once

#include <mgcom/rma/untyped.h>

#ifdef __cplusplus
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom::rma::local_pointer<type>
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom::rma::remote_pointer<type>
#else
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom_rma_local_address
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom_rma_remote_address
#endif

