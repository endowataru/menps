
#pragma once

#include <mgcom.hpp>
#include "common/rma/rma.hpp"
#include <mgbase/unique_ptr.hpp>
#include "device/fjmpi/fjmpi_interface.hpp"

namespace mgcom {
namespace fjmpi {

class command_producer;

mgbase::unique_ptr<rma::requester> make_rma_requester(command_producer&, rpc::requester&);

mgbase::unique_ptr<rma::registrator> make_rma_registrator(fjmpi_interface&);

} // namespace fjmpi
} // namespace mgcom

