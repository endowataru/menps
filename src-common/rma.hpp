
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace rma {

void initialize();

void finalize();

bool try_compare_and_swap_64(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* expected
,   const mgbase::uint64_t* desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

bool try_fetch_and_add_64(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* value
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

}

}


