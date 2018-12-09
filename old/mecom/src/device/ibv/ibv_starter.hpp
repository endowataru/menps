
#pragma once

#include "common/starter.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

mefdn::unique_ptr<starter> make_starter(int* argc, char*** argv);

} // namespace ibv
} // namespace mecom
} // namespace menps

