
#pragma once

#include <menps/mecom/collective/requester.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include "device/mpi3/mpi3_interface.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

mefdn::unique_ptr<mecom::collective::requester> make_collective_requester(mpi3_interface&);

} // namespace mpi3
} // namespace mecom
} // namespace menps

