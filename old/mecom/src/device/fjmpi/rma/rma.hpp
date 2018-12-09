
#pragma once

#include <menps/mecom.hpp>
#include "common/rma/rma.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>
#include "device/fjmpi/fjmpi_interface.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class command_producer;

mefdn::unique_ptr<rma::requester> make_rma_requester(command_producer&, rpc::requester&);

mefdn::unique_ptr<rma::registrator> make_rma_registrator(fjmpi_interface&);

} // namespace fjmpi
} // namespace mecom
} // namespace menps

