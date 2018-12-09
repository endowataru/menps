
#pragma once

#include "common/starter.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

mefdn::unique_ptr<starter> make_starter(int* const argc, char*** const argv);

} // namespace fjmpi
} // namespace mecom
} // namespace menps

