
#pragma once

#include <mgcom.hpp>

namespace mgcom {
namespace am {
namespace sender {

void initialize();

void finalize();

void add_ticket_to(process_id_t dest_proc, index_t ticket);

} // namespace sender
} // namespace am
} // namespace mgcom

