
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace {

struct am_message {
    am_handler_id_t id;
    index_t         size;
    mgbase::uint8_t data[4096 - sizeof(am_handler_id_t) - sizeof(index_t)];
};

}

}

