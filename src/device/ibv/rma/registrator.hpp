
#pragma once

#include <mgcom/rma/registrator.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class endpoint;

mgbase::unique_ptr<rma::registrator> make_rma_registrator(endpoint&);

} // namespace ibv
} // namespace mgcom

