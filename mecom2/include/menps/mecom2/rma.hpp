
#pragma once

#include <menps/mecom2/rma/single/single_rma.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/rma/uct/uct_rma.hpp>
#if 0
#include <menps/mecom2/rma/ucp/ucp_rma.hpp>
#endif

namespace menps {
namespace mecom2 {

enum class rma_id_t
{
    single = 1
,   mpi
,   uct
#if 0
,   ucp
#endif
};

template <rma_id_t Id, typename MpiItf>
struct get_rma_type;

template <typename MpiItf>
struct get_rma_type<rma_id_t::single, MpiItf>
    : mefdn::type_identity<single_rma> { };

template <typename MpiItf>
struct get_rma_type<rma_id_t::mpi, MpiItf>
    : mefdn::type_identity<mpi_rma<mpi_rma_policy<MpiItf>>> { };

template <typename MpiItf>
struct get_rma_type<rma_id_t::uct, MpiItf>
    : mefdn::type_identity<uct_rma<typename MpiItf::ult_itf_type>> { };

#if 0
template <typename MpiItf>
struct get_rma_type<rma_id_t::ucp, MpiItf>
    : mefdn::type_identity<ucp_rma> { };
#endif

template <rma_id_t Id, typename MpiItf>
using get_rma_type_t = typename get_rma_type<Id, MpiItf>::type;

} // namespace menps
} // namespace mecom2

