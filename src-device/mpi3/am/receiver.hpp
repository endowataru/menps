
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace am {
namespace receiver {

void initialize();

void finalize();

void poll();

index_t get_tickets(process_id_t src_proc);

struct constants {
    static const index_t max_num_tickets = 32;
};

}
}

}

