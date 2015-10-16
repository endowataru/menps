
#pragma once

#include <mgcom.hpp>

namespace mgcom {
namespace am {

struct constants {
    static const index_t max_num_tickets = 32;
};

struct message_buffer {
    message msgs[constants::max_num_tickets];
};

}
}

