
#pragma once

#include <mgbase/unique_ptr.hpp>
#include "common/starter.hpp"

namespace mgcom {
namespace mpi1 {

mgbase::unique_ptr<starter> make_starter(int* argc, char*** argv);

} // namespace mpi1
} // namespace mgcom

