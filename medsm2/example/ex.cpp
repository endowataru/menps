
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    using namespace menps;
    
    auto mi =
        mefdn::make_unique<medev2::mpi::direct_requester>(&argc, &argv);
    
    auto coll = mecom2::make_mpi_coll(*mi, MPI_COMM_WORLD);
    
    auto rma_info = medsm2::make_dsm_rma_info<mecom2::rma_id_t::MEDSM2_COM_RMA>(*mi, coll);
    auto& rma = rma_info->rma;
    
    auto p2p = mecom2::make_mpi_p2p(*mi, MPI_COMM_WORLD);
    
    medsm2::default_dsm_com_itf com(medsm2::default_dsm_com_itf::conf_t{ *rma, coll, p2p });
    
    medsm2::mpi_svm_space sp(com);
    
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


