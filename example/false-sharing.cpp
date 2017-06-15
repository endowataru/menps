
#include <mgcom.hpp>
#include <mgdsm.hpp>
#include <mgdsm/segment_ref.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        sp.enable_on_this_thread();
        
        mgdsm::segment_ref seg;
        
        int* pi = MGBASE_NULLPTR;
        if (mgcom::current_process_id() == 0) {
            seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
            pi = static_cast<int*>(seg.get_ptr());
        }
        
        mgcom::collective::broadcast(0, &pi, 1);
        
        for (int i = 0; i < 1000; ++i) {
            ++pi[mgcom::current_process_id()];
            sp.write_barrier();
        }
        
        mgcom::collective::barrier();
        
        sp.read_barrier();
        
        for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        {
            std::cout << mgcom::current_process_id() << " " << pi[proc] << std::endl;
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mgcom::finalize();
    
    return 0;
}
