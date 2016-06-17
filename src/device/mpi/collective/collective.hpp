
#pragma once

#include <mgcom/collective.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi {
namespace collective {

using namespace mgcom::collective;

mgbase::unique_ptr<requester> make_requester();

namespace untyped {

using namespace mgcom::collective::untyped;

void broadcast(const broadcast_params& params);

void allgather(const allgather_params& params);

void alltoall(const alltoall_params& params);

} // namespace untyped

} // namespace collective
} // namespace mpi
} // namespace mgcom

