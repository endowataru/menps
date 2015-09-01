
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace am {
namespace sender {

void initialize();

void finalize();

void release_resource_at(process_id_t proc);

}
}

}

