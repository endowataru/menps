
#pragma once

#include "common/starter.hpp"

namespace mgcom {
namespace fjmpi {

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv);

} // namespace fjmpi
} // namespace mgcom

