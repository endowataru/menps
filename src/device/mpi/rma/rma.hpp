
#pragma once

#include <mgcom/rma.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi {
namespace rma {

using namespace mgcom::rma;

namespace untyped = mgcom::rma::untyped;

mgbase::unique_ptr<requester> make_requester();

mgbase::unique_ptr<registrator> make_registrator();

} // namespace rma
} // namespace mpi
} // namespace mgcom

