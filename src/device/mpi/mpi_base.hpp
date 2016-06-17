
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/unique_ptr.hpp>

#include "mpi.hpp"

#include "mpi_error.hpp"

namespace mgcom {
namespace mpi {

mgbase::unique_ptr<endpoint> make_endpoint(int* argc, char*** argv);

inline bool is_valid_rank(const int rank)
{
    return valid_process_id(static_cast<process_id_t>(rank));
}

} // namespace mpi
} // namespace mgcom

