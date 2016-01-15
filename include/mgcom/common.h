
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <mgbase/deferred.h>

MGBASE_EXTERN_C_BEGIN

typedef uint64_t  mgcom_index_t;

typedef uint32_t  mgcom_process_id_t;

typedef enum mgcom_error_code {
    MGCOM_SUCCESS
,   MGCOM_FAILURE
}
mgcom_error_code_t;

/**
 * Initialize and start the communication.
 */
mgcom_error_code_t mgcom_initialize(int* argc, char*** argv) MGBASE_NOEXCEPT;

/**
 * Finalize the communication.
 */
mgcom_error_code_t mgcom_finalize(void) MGBASE_NOEXCEPT;


mgcom_process_id_t mgcom_current_process_id(void) MGBASE_NOEXCEPT;

mgcom_index_t mgcom_number_of_processes(void) MGBASE_NOEXCEPT;

MGBASE_EXTERN_C_END

