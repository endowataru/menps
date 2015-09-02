
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace am {
namespace sender {

void initialize();

void finalize();

void add_ticket(process_id_t src_proc, index_t ticket);

}
}

}

