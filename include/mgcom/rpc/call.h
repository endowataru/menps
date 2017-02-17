
#pragma once

#include <mgcom/common.h>

MGBASE_EXTERN_C_BEGIN


typedef mgcom_index_t  mgcom_rpc_handler_id_t;

typedef struct mgcom_rpc_handler_parameters {
    mgcom_process_id_t  source;
    const void*         data;
    mgcom_index_t       size;
    void*               result;
}
mgcom_rpc_handler_parameters;

typedef mgcom_index_t (*mgcom_rpc_handler_function_t)(void*, const mgcom_rpc_handler_parameters*);

MGBASE_EXTERN_C_END

