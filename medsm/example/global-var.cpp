
#include <menps/mecom.hpp>
#include <menps/medsm.hpp>
#include <iostream>

#ifdef __APPLE__
int main() { }

#else

MEDSM_GLOBAL_VARIABLE int g_val;

int main(int argc, char* argv[])
{
    mecom::initialize(&argc, &argv);
    
    {
        auto sp = medsm::make_space();
        
        sp.enable_on_this_thread();
        
        if (mecom::current_process_id() == 0) {
            g_val = 123;
        
            sp.write_barrier();
        }
        
        mecom::collective::barrier();
        
        sp.read_barrier();
        
        std::cout << mecom::current_process_id() << " " << g_val << std::endl;
    }
    
    mecom::finalize();
    
    return 0;
}
#endif

