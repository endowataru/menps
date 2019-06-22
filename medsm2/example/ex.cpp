
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    using namespace menps;
    
    medsm2::dsm_com_itf_t com(&argc, &argv);
    auto& coll = com.get_coll();
    
    medsm2::mpi_svm_space sp(com);
    
    auto p = sp.coll_alloc_seg(1<<15, 1<<12);
    
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


