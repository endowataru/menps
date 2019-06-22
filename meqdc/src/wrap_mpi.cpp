
#include <menps/meqdc/mpi/proxy_mpi_itf.hpp>
#include <menps/medev2/mpi/direct_mpi_facade.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/ult_ext_itf.hpp>

#include MEFDN_PP_CAT(CMPTH_ULT_HEADER_, MEQDC_MPI_ULT_ITF)

namespace /*unnamed*/ {

using proxy_mpi_itf_t =
    menps::meqdc::proxy_mpi_itf<
        menps::medev2::mpi::direct_mpi_itf<
            cmpth::get_ult_ext_itf_t<cmpth::ult_tag_t::MEQDC_MPI_ULT_ITF>
        >
    >;

proxy_mpi_itf_t::mpi_facade_type* g_mf;

} // unnamed namespace

#define D(prefix, mf, name, Name, tr, num, ...) \
    tr prefix##Name( \
        MEDEV2_EXPAND_PARAMS_TO_PARAMS(num, __VA_ARGS__) \
    ) { \
        (mf).name({ \
            MEDEV2_EXPAND_PARAMS_TO_ARGS(num, __VA_ARGS__) \
        }); \
        return MPI_SUCCESS; \
    }

extern "C" {

int MPI_Init(
    int* const      argc
,   char*** const   argv
) {
    int provided = 0;
    g_mf = new proxy_mpi_itf_t::mpi_facade_type(argc, argv, MPI_THREAD_SINGLE, &provided);
    return MPI_SUCCESS;
}

int MPI_Init_thread(
    int* const      argc
,   char*** const   argv
,   const int       required
,   int* const      provided
) {
    g_mf = new proxy_mpi_itf_t::mpi_facade_type(argc, argv, required, provided);
    return MPI_SUCCESS;
}

int MPI_Finalize()
{
    delete g_mf;
    return MPI_SUCCESS;
}

MEDEV2_MPI_FUNCS_ALL(D, MPI_, *g_mf)

} // extern "C"

#undef D

