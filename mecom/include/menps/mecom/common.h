
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <menps/mecom/config.h>

#include <menps/mefdn/lang.hpp> // TODO: C++

MEFDN_EXTERN_C_BEGIN

typedef uint64_t  mecom_index_t;

typedef uint32_t  mecom_process_id_t;

typedef enum mecom_error_code {
    MECOM_SUCCESS
,   MECOM_FAILURE
}
mecom_error_code_t;

#if 0

/**
 * Initialize and start the communication.
 */
mecom_error_code_t mecom_initialize(int* argc, char*** argv) MEFDN_NOEXCEPT;

/**
 * Finalize the communication.
 */
mecom_error_code_t mecom_finalize(void) MEFDN_NOEXCEPT;


mecom_process_id_t mecom_current_process_id(void) MEFDN_NOEXCEPT;

mecom_index_t mecom_number_of_processes(void) MEFDN_NOEXCEPT;

#endif

MEFDN_EXTERN_C_END

