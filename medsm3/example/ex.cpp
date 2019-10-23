
#include <menps/medsm3/md3/make_md3_space.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    using namespace menps;
    
    medsm2::dsm_com_itf_t com(&argc, &argv);
    auto& coll = com.get_coll();
    
    auto sp = medsm3::make_md3_space(com);

    sp->enable_on_this_thread();

    auto p = sp->coll_alloc_seg(1<<15, 1<<12);

    if (coll.this_proc_id() == 0) {
        *(int*)p = 1;
    }
    
    sp->barrier();
    
    std::cout << coll.this_proc_id() << " " << *(int*)p << std::endl;
    
    coll.barrier();
    
    return 0;
}

