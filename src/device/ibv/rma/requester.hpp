
#pragma once

#include "device/ibv/command/completer.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class endpoint;
class completer;

mgbase::unique_ptr<rma::requester> make_rma_direct_requester(endpoint&, completer&);

} // namespace ibv
} // namespace mgcom

