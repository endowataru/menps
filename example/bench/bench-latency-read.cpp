
#include <mgcom.hpp>
#include <mgcom_collective.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    using namespace mgcom;
    using namespace mgcom::rma;
    
    initialize(&argc, &argv);
    
    int x = 0;
    const local_region local_reg = register_region(&x, sizeof(x));
    const local_address local_addr = to_address(local_reg);
    
    const process_id_t root_proc = 0;
    local_address root_local_addr = local_addr;
    mgcom::collective::broadcast(root_proc, &root_local_addr, 1);
    
    const remote_address root_remote_addr = use_remote_address(root_proc, root_local_addr);
    
    double clocks = 0;
    const int number_of_trials = atoi(argv[1]);
    
    if (current_process_id() == root_proc)
        x = 123;
    
    barrier();
    
    if (current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            x = 0;
            
            mgbase::stopwatch sw;
            sw.start();
            
            remote_read_cb cb;
            remote_read_nb(cb, root_proc, root_remote_addr, local_addr, sizeof(x));
            mgbase::control::wait(cb);
            
            clocks += sw.elapsed();
            
            if (x != 123)
                std::cout << "error!" << std::endl;
        }
    }
    
    barrier();
    
    for (process_id_t proc = 0; proc < number_of_processes(); ++proc) {
        if (proc == current_process_id())
            std::cout << current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        barrier();
    }
    
    finalize();
    
    return 0;
}

