
#pragma once

#include <mgcom/collective/requester.hpp>
#include <mgbase/unique_ptr.hpp>
#include "device/mpi3/mpi3_interface.hpp"

namespace mgcom {
namespace mpi3 {

mgbase::unique_ptr<mgcom::collective::requester> make_collective_requester(mpi3_interface&);

} // namespace mpi3
} // namespace mgcom

