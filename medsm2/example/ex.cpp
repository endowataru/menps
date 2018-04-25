
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
    
    auto rma = mecom2::make_mpi_rma(*mi, win);
    auto coll = mecom2::make_mpi_coll(*mi, MPI_COMM_WORLD);
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


