#pragma once

#include <menps/mefdn/memory/unique_ptr.hpp>
#include "common/starter.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

mefdn::unique_ptr<starter> make_starter(int* argc, char*** argv);

} // namespace mpi3
} // namespace mecom
} // namespace menps

