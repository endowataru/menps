
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

enum class rma_itf_id_t
{
    SINGLE = 1
,   MPI
,   UCT
};

#define MECOM2_RMA_ITF_HEADER_SINGLE <menps/mecom2/rma/single/single_rma.hpp>
#define MECOM2_RMA_ITF_HEADER_MPI    <menps/mecom2/rma/mpi/mpi_rma.hpp>
#define MECOM2_RMA_ITF_HEADER_UCT    <menps/mecom2/rma/uct/uct_rma.hpp>

template <rma_itf_id_t Id, typename P>
struct get_rma_itf_type;

template <rma_itf_id_t Id, typename P>
using get_rma_itf_type_t = typename get_rma_itf_type<Id, P>::type;

} // namespace menps
} // namespace mecom2

