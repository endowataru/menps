
#pragma once

#include "mpi.hpp"
#include "mpi_error.hpp"

#include "endpoint.hpp"

namespace mgcom {
namespace mpi {

inline bool is_valid_rank(endpoint& ep, const int rank)
{
    return valid_process_id(ep, static_cast<process_id_t>(rank));
}

std::string get_comm_name(const MPI_Comm comm);

} // namespace mpi
} // namespace mgcom

