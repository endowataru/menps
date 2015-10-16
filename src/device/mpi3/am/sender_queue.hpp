
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace am {
namespace sender_queue {

void initialize();

void finalize();

void enqueue();

void poll();

}
}

}

