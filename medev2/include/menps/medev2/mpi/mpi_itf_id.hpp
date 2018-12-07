
#pragma once

#include <menps/medev2/common.hpp>

namespace menps {
namespace medev2 {

enum class mpi_itf_id_t {
    DIRECT = 1
,   QDC
};

template <mpi_itf_id_t Id, typename UltItf>
struct get_mpi_itf_type;

template <mpi_itf_id_t Id, typename UltItf>
using get_mpi_itf_type_t = typename get_mpi_itf_type<Id, UltItf>::type;

#define MEDEV2_MPI_ITF_HEADER_DIRECT    <menps/medev2/mpi/direct_mpi_facade.hpp>
#define MEDEV2_MPI_ITF_HEADER_QDC       <menps/meqdc/mpi.hpp>

} // namespace medev2
} // namespace menps

