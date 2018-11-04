
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mecom2/rma.hpp>

namespace menps {
namespace medsm2 {

template <mecom2::rma_id_t Id, typename MpiItf>
struct dsm_rma_info;

template <typename MpiItf>
struct dsm_rma_info<mecom2::rma_id_t::single, MpiItf>
{
    using mpi_facade_type = typename MpiItf::mpi_facade_type;
    
    mecom2::single_rma_ptr  rma;
    
    template <typename Coll>
    explicit dsm_rma_info(mpi_facade_type& /*mf*/, Coll& /*coll*/)
    {
        rma = mecom2::make_single_rma();
    }
};

template <typename MpiItf>
struct dsm_rma_info<mecom2::rma_id_t::mpi, MpiItf>
{
    using mpi_facade_type = typename MpiItf::mpi_facade_type;
    using mpi_rma_policy_type = mecom2::mpi_rma_policy<MpiItf>;
    
    MPI_Comm    comm;
    MPI_Win     win;
    mecom2::mpi_rma_ptr<mpi_rma_policy_type> rma;
    
    template <typename Coll>
    explicit dsm_rma_info(mpi_facade_type& mf, Coll& /*coll*/)
    {
        mf.comm_dup({ MPI_COMM_WORLD, &comm }); // TODO
        mf.win_create_dynamic({ MPI_INFO_NULL, comm, &win });
        
        mf.win_lock_all({ 0, win });
        
        rma = mecom2::make_mpi_rma<mpi_rma_policy_type>(mf, win, MPI_COMM_WORLD);
    }
};

template <typename MpiItf>
struct dsm_rma_info<mecom2::rma_id_t::uct, MpiItf>
{
    using mpi_facade_type = typename MpiItf::mpi_facade_type;
    using ult_itf_type = typename MpiItf::ult_itf_type;
    
    mefdn::unique_ptr<mecom2::uct_rma_resource<ult_itf_type>> rma_res;
    mecom2::uct_rma<ult_itf_type>*                    rma;
    
    template <typename Coll>
    explicit dsm_rma_info(mpi_facade_type& /*mf*/, Coll& coll)
    {
        // TODO
        const char tl_name[] = "rc_mlx5";
        //const char tl_name[] = "rc";
        const char dev_name[] = "mlx5_0:1";
        
        rma_res = mecom2::make_uct_rma_resource(tl_name, dev_name, coll);
        rma = rma_res->rma.get();
    }
};

#if 0
template <typename MpiItf>
struct dsm_rma_info<mecom2::rma_id_t::ucp, MpiItf>
{
    using mpi_facade_type = typename MpiItf::mpi_facade_type;
    
    mecom2::rma_ucp_policy::ucp_facade_type     uf;
    mecom2::rma_ucp_policy::context_type        ctx;
    mefdn::unique_ptr<mecom2::ucp_worker_set>   wk_set;
    mecom2::ucp_rma_ptr                         rma;
    
    template <typename Coll>
    explicit dsm_rma_info(mpi_facade_type& /*mf*/, Coll& coll)
    {
        using mecom2::rma_ucp_policy;
        
        auto conf = rma_ucp_policy::config_type::read(uf, nullptr, nullptr);
        
        ucp_params ctx_params = ucp_params();
        ctx_params.field_mask = UCP_PARAM_FIELD_FEATURES;
        ctx_params.features   = UCP_FEATURE_RMA | UCP_FEATURE_AMO64;
        
        ctx = rma_ucp_policy::context_type::init(uf, &ctx_params, conf.get());
        
        ucp_worker_params_t wk_params = ucp_worker_params_t();
        wk_set = mecom2::make_ucp_worker_set(uf, ctx, wk_params, coll);
        
        rma = mecom2::make_ucp_rma(uf, ctx, *wk_set);
    }
};
#endif

template <mecom2::rma_id_t Id, typename MpiItf, typename Coll>
inline mefdn::unique_ptr<dsm_rma_info<Id, MpiItf>>
make_dsm_rma_info(typename MpiItf::mpi_facade_type& mf, Coll& coll)
{
    return mefdn::make_unique<dsm_rma_info<Id, MpiItf>>(mf, coll);
}

} // namespace medsm2
} // namespace menps

