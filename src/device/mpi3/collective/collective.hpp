
#pragma once

#include <mgcom/collective/requester.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi3 {
namespace collective {

using namespace mgcom::collective;

mgbase::unique_ptr<requester> make_requester();

} // namespace collective
} // namespace mpi3
} // namespace mgcom

