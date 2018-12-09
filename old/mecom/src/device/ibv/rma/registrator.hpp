
#pragma once

#include <menps/mecom/rma/registrator.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class endpoint;

mefdn::unique_ptr<rma::registrator> make_rma_registrator(endpoint&);

} // namespace ibv
} // namespace mecom
} // namespace menps

