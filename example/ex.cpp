
#include <mgcom.hpp>
#include <mgdsm.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        auto seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
        
        sp.enable_on_this_thread();
        
        int* pi = nullptr;
        
        if (mgcom::current_process_id() == 0)
        {
            pi = static_cast<int*>(seg.get_ptr());
        }
        
        mgcom::collective::broadcast(0, &pi, 1);
        
        if (mgcom::current_process_id() == 0)
        {
            *pi = 123;
            
            sp.write_barrier();
        }
        
        mgcom::collective::barrier();
        
        sp.read_barrier();
        
        for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        {
            if (proc == mgcom::current_process_id())
            {
                std::cout << mgcom::current_process_id() << " " << *pi << std::endl;
            }
            
            mgcom::collective::barrier();
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mgcom::finalize();
    
    return 0;
}

