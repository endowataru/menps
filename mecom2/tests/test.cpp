
#include <menps/mecom2/com/mpi/mpi_com_itf.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/ult_ext_itf.hpp>

#define MECOM2_TEST_ULT_ITF     SCT
#define MECOM2_TEST_MPI_ITF     QDC
#define MECOM2_TEST_RMA_ITF     MPI

#include MEFDN_PP_CAT(CMPTH_ULT_HEADER_, MECOM2_TEST_ULT_ITF)
#include MEFDN_PP_CAT(MEDEV2_MPI_ITF_HEADER_, MECOM2_TEST_MPI_ITF)

namespace /*unnamed*/ {

struct com_policy
{
    using ult_itf_type =
        cmpth::get_ult_ext_itf_t<
            cmpth::ult_tag_t::MECOM2_TEST_ULT_ITF
        ,   cmpth::sync_tag_t::MCS // TODO
        >;
    
    using mpi_itf_type =
        menps::medev2::get_mpi_itf_type_t<
            menps::medev2::mpi_itf_id_t::MECOM2_TEST_MPI_ITF
        ,   ult_itf_type
        >;
    
    static constexpr menps::mecom2::rma_itf_id_t rma_id =
        menps::mecom2::rma_itf_id_t::MECOM2_TEST_RMA_ITF;
};

using com_itf_type = menps::mecom2::mpi_com_itf<com_policy>;
using coll_itf_type = com_itf_type::coll_itf_type;
using rma_itf_type = com_itf_type::rma_itf_type;
using proc_id_type = com_itf_type::proc_id_type;

com_itf_type* g_com;
coll_itf_type* g_coll;
rma_itf_type* g_rma;

} // unnamed namespace

#include "test.hpp"

int main(int argc, char* argv[])
{
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    
    com_itf_type com{&argc, &argv};
    g_com = &com;
    g_coll = &com.get_coll();
    g_rma = &com.get_rma();
    
    int res = context.run();
    
    return res;
}

