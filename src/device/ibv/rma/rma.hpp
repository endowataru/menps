
#pragma once

#include <mgcom.hpp>
#include "common/rma/rma.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {
namespace rma {

using namespace mgcom::rma;

namespace untyped {

using namespace mgcom::rma::untyped;

bool try_read_async(const untyped::read_params& params);

bool try_write_async(const untyped::write_params& params);

} // namespace untyped

bool try_atomic_read_async(const rma::atomic_read_params<atomic_default_t>& params);

bool try_atomic_write_async(const rma::atomic_write_params<atomic_default_t>& params);

bool try_compare_and_swap_async(const rma::compare_and_swap_params<atomic_default_t>& params);

bool try_fetch_and_add_async(const rma::fetch_and_add_params<atomic_default_t>& params);

namespace untyped {

untyped::local_region register_region(const untyped::register_region_params& params);

void deregister_region(const untyped::deregister_region_params& params);

untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params);

} // namespace untyped

mgbase::unique_ptr<requester> make_requester();

mgbase::unique_ptr<registrator> make_registrator();

} // namespace rma
} // namespace ibv
} // namespace mgcom

