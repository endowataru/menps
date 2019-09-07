
#pragma once

#include <menps/medsm2/common.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/ult_ext_itf.hpp>
#include <menps/mecom2/com/mpi/mpi_com_itf.hpp>
#include <menps/medsm2/prof_aspect.hpp>

#include MEFDN_PP_CAT(CMPTH_ULT_HEADER_, MEDSM2_ULT_ITF)
#include MEFDN_PP_CAT(MEDEV2_MPI_ITF_HEADER_, MEDSM2_MPI_ITF)
#ifdef MEDEV2_DEVICE_UCX_ENABLED
    #include MEFDN_PP_CAT(MEDEV2_UCT_ITF_HEADER_, MEDSM2_UCT_ITF)
#endif
#include MEFDN_PP_CAT(CMPTH_PROF_HEADER_, MEDSM2_PROF_ASPECT)

namespace menps {
namespace medsm2 {

struct dsm_com_policy_base
{
    using ult_itf_type =
        cmpth::get_ult_ext_itf_t<cmpth::ult_tag_t::MEDSM2_ULT_ITF, cmpth::sync_tag_t::MEDSM2_DELEGATOR_ITF>;
    
    using mpi_itf_type =
        medev2::get_mpi_itf_type_t<
            medev2::mpi_itf_id_t::MEDSM2_MPI_ITF
        ,   ult_itf_type
        >;
    
    #ifdef MEDEV2_DEVICE_UCX_ENABLED
    using uct_itf_type =
        medev2::ucx::uct::get_uct_itf_type_t<
            medev2::ucx::uct::uct_itf_id_t::MEDSM2_UCT_ITF
        ,   dsm_com_policy_base
        >;
    #endif
    
    static constexpr mecom2::rma_itf_id_t rma_id = mecom2::rma_itf_id_t::MEDSM2_RMA_ITF;
    
    using prof_aspect_type =
        typename ult_itf_type::template prof_aspect_t<
            cmpth::prof_tag::MEDSM2_PROF_ASPECT, prof_aspect_policy>;
};

using dsm_com_itf_t = mecom2::mpi_com_itf<dsm_com_policy_base>;

} // namespace medsm2
} // namespace menps

