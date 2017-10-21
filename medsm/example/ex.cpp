
#include <menps/mecom.hpp>
#include <menps/medsm.hpp>
#include <iostream>

namespace mecom = menps::mecom;
namespace medsm = menps::medsm;

int main(int argc, char* argv[])
{
    mecom::initialize(&argc, &argv);
    
    {
        auto sp = medsm::make_space();
        
        auto seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
        
        sp.enable_on_this_thread();
        
        int* pi = nullptr;
        
        if (mecom::current_process_id() == 0)
        {
            pi = static_cast<int*>(seg.get_ptr());
        }
        
        mecom::collective::broadcast(0, &pi, 1);
        
        if (mecom::current_process_id() == 0)
        {
            *pi = 123;
            
            sp.write_barrier();
        }
        
        mecom::collective::barrier();
        
        sp.read_barrier();
        
        for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc)
        {
            if (proc == mecom::current_process_id())
            {
                std::cout << mecom::current_process_id() << " " << *pi << std::endl;
            }
            
            mecom::collective::barrier();
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mecom::finalize();
    
    return 0;
}

