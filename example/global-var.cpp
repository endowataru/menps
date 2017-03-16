
#include <mgcom.hpp>
#include <mgdsm.hpp>
#include <iostream>

MGDSM_GLOBAL_VARIABLE int g_val;

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        sp.enable_on_this_thread();
        
        if (mgcom::current_process_id() == 0) {
            g_val = 123;
        
            sp.write_barrier();
        }
        
        mgcom::collective::barrier();
        
        sp.read_barrier();
        
        std::cout << mgcom::current_process_id() << " " << g_val << std::endl;
    }
    
    mgcom::finalize();
    
    return 0;
}

