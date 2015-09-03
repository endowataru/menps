
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace rma {

void initialize();

void finalize();

bool try_compare_and_swap_64_async(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* expected
,   const mgbase::uint64_t* desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
);

}

}


