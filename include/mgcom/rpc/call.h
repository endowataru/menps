
#pragma once

#include <mgcom/common.h>

MGBASE_EXTERN_C_BEGIN

#define MGCOM_RPC_MAX_DATA_SIZE         1024
#define MGCOM_RPC_MAX_NUM_HANDLERS      10000

typedef mgcom_index_t  mgcom_rpc_handler_id_t;

typedef struct mgcom_rpc_handler_parameters {
    mgcom_process_id_t  source;
    const void*         data;
    mgcom_index_t       size;
    void*               result;
}
mgcom_rpc_handler_parameters;

typedef mgcom_index_t (*mgcom_rpc_handler_function_t)(const mgcom_rpc_handler_parameters*);

MGBASE_EXTERN_C_END

