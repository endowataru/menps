
#pragma once

#include <mgcom.hpp>
#include "common/rma/rma.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace fjmpi {
namespace rma {

using namespace mgcom::rma;

mgbase::unique_ptr<requester> make_requester();

mgbase::unique_ptr<registrator> make_registrator();

namespace untyped {

using namespace mgcom::rma::untyped;

bool try_read_async(const untyped::read_params& params);

bool try_write_async(const untyped::write_params& params);

bool try_remote_read_async_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   int                         flags
,   const mgbase::operation&    on_complete
);

bool try_remote_write_async_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   int                         flags
,   const mgbase::operation&    on_complete
);

} // namespace untyped

struct constants {
    static const int max_nic_count = 4;
};

} // namespace rma
} // namespace fjmpi
} // namespace mgcom

