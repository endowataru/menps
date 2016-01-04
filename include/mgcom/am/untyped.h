
#pragma once

#include <mgcom/common.h>

MGBASE_EXTERN_C_BEGIN

/**
 * Type of Unique IDs of Active Messages' handler.
 */
typedef mgcom_index_t  mgcom_am_handler_id_t;

typedef struct mgcom_callback_argument {
    mgcom_process_id_t source;
    const void*        data;
    mgcom_index_t      size;
}
mgcom_am_callback_parameters;

/**
 * Callback function of Active Messages' handler.
 */
typedef void (*mgcom_am_handler_callback_t)(const mgcom_am_callback_parameters*);

/**
 * Register a callback function as a Active Messages' handler.
 */
mgcom_error_code_t mgcom_am_register_handler(
    mgcom_am_handler_id_t
,   mgcom_am_handler_callback_t
);

#define MGCOM_AM_MAX_DATA_SIZE 1024
#define MGCOM_AM_HANDLE_SIZE   1024

typedef struct mgcom_am_am_message_buffer {
    mgcom_am_handler_id_t   id;
    mgcom_index_t           size;
    mgcom_index_t           ticket;
    uint8_t                 data[MGCOM_AM_MAX_DATA_SIZE]; // TODO
}
mgcom_am_am_message_buffer;

/// Control block for sending Active Messages.
typedef struct mgcom_am_send_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          dest_proc;
    
    // The message body is copied to the local buffer.
    mgcom_am_am_message_buffer  msg;
    
    uint8_t                     handle[MGCOM_AM_HANDLE_SIZE];
}
mgcom_am_send_cb;

/**
 * Invoke the callback function on the specified remote node.
 */
mgcom_error_code_t mgcom_am_send(
    mgcom_am_send_cb*     cb
,   mgcom_am_handler_id_t id
,   const void*           value
,   mgcom_index_t         size
,   mgcom_process_id_t    dest_proc
);

MGBASE_EXTERN_C_END

