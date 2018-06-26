
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    using namespace menps;
    
    auto mi =
        mefdn::make_unique<medev2::mpi::direct_requester>(&argc, &argv);
    
    /*const*/ auto win =
        mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    mi->win_lock_all({ 0, win });
    
    auto coll = mecom2::make_mpi_coll(*mi, MPI_COMM_WORLD);
    
    #if defined(MEDSM2_USE_UCT_RMA)
    // TODO
    const char tl_name[] = "rc_mlx5";
    const char dev_name[] = "mlx5_0:1";
    auto rma_res = mecom2::make_uct_rma_resource(tl_name, dev_name, coll);
    auto* rma = rma_res->rma.get();
    
    #elif defined(MEDSM2_USE_UCP_RMA)
    using mecom2::rma_ucp_policy;
    rma_ucp_policy::ucp_facade_type uf;
    
    auto conf = rma_ucp_policy::config_type::read(uf, nullptr, nullptr);
    
    ucp_params ctx_params = ucp_params();
    ctx_params.field_mask = UCP_PARAM_FIELD_FEATURES;
    ctx_params.features   = UCP_FEATURE_RMA;
    
    auto ctx = rma_ucp_policy::context_type::init(uf, &ctx_params, conf.get());
    
    ucp_worker_params_t wk_params = ucp_worker_params_t();
    auto wk_set = mecom2::make_ucp_worker_set(uf, ctx, wk_params, coll);
    
    auto rma = mecom2::make_ucp_rma(uf, ctx, *wk_set);
    
    #else
    auto rma = mecom2::make_mpi_rma(*mi, win, MPI_COMM_WORLD);
    #endif
    auto p2p = mecom2::make_mpi_p2p(*mi, MPI_COMM_WORLD);
    
    medsm2::mpi_svm_space sp(*rma, coll, p2p);
    
    auto p = sp.coll_alloc_seg(1<<15, 1<<12);
    
    //medsm2::mpi_svm_space::rel_id ri{};
    
    if (coll.this_proc_id() == 0) {
        *(int*)p = 1;
        //sp.store_release(p);
    }
    
    //coll.barrier();
    //coll.broadcast(0, &ri, 1);
    
    //sp.load_acquire(p);
    
    sp.barrier();
    
    std::cout << coll.this_proc_id() << " " << *(int*)p << std::endl;
    
    coll.barrier();
    
    return 0;
}


