
#pragma once

#include <mgcom/common.h>
#include <mgbase/deferred.hpp>

#include <cstddef>
#include <cstring>

namespace mgcom {

typedef mgcom_index_t                  index_t;

typedef mgcom_process_id_t             process_id_t;


/**
 * Initialize and start the communication.
 */
void initialize(int* argc, char*** argv);

/**
 * Finalize the communication.
 */
void finalize();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;

} // namespace mgcom

