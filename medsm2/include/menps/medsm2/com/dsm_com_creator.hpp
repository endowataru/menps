
#pragma once

#include <menps/medsm2/common.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/ult_ext_itf.hpp>
#ifdef MEFDN_ENABLE_MEULT
    #include <menps/meult/ult_itf_id.hpp>
#endif
#include <menps/mecom2/com/mpi/mpi_com_itf.hpp>
#include <menps/mefdn/profiling/clock.hpp> // get_cpu_clock

#ifdef MEFDN_ENABLE_MEULT
    #include MEFDN_PP_CAT(MEULT_ULT_ITF_HEADER_, MEDSM2_ULT_ITF)
#else
    #include MEFDN_PP_CAT(CMPTH_ULT_HEADER_, MEDSM2_ULT_ITF)
#endif
#include MEFDN_PP_CAT(MEDEV2_MPI_ITF_HEADER_, MEDSM2_MPI_ITF)
#include MEFDN_PP_CAT(MEDEV2_UCT_ITF_HEADER_, MEDSM2_UCT_ITF)

namespace menps {
namespace medsm2 {

struct dsm_com_policy_base
{
    using ult_itf_type =
        #ifdef MEFDN_ENABLE_MEULT
        meult::get_ult_itf_type_t<meult::ult_itf_id_t::MEDSM2_ULT_ITF>;
        #else
        cmpth::get_ult_ext_itf_t<cmpth::ult_tag_t::MEDSM2_ULT_ITF>;
        #endif
    
    using mpi_itf_type =
        medev2::get_mpi_itf_type_t<
            medev2::mpi_itf_id_t::MEDSM2_MPI_ITF
        ,   ult_itf_type
        >;
    
    using uct_itf_type =
        medev2::ucx::uct::get_uct_itf_type_t<
            medev2::ucx::uct::uct_itf_id_t::MEDSM2_UCT_ITF
        ,   dsm_com_policy_base
        >;
    
    static constexpr mecom2::rma_itf_id_t rma_id = mecom2::rma_itf_id_t::MEDSM2_RMA_ITF;
};

using dsm_com_creator = mecom2::mpi_com_itf<dsm_com_policy_base>;

} // namespace medsm2
} // namespace menps

