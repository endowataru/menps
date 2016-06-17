
#pragma once

#include "common/starter.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

mgbase::unique_ptr<starter> make_starter(int* argc, char*** argv);

void initialize();

void finalize();

} // namespace ibv
} // namespace mgcom

