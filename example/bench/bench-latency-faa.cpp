
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t>
        local_ptr = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
    
    *local_ptr = 0;
    
    const mgcom::process_id_t root_proc = 0;
    mgcom::collective::broadcast(root_proc, &local_ptr, 1);
    
    mgcom::rma::remote_ptr<mgcom::rma::atomic_default_t>
        remote_ptr = mgcom::rma::use_remote_ptr(root_proc, local_ptr);
    
    double clocks = 0;
    const int number_of_trials = atoi(argv[1]);
    
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t>
        value_ptr = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
    
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t>
        result_ptr = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
    
    *value_ptr = 1;
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mgbase::stopwatch sw;
            sw.start();
            
            mgcom::rma::remote_fetch_and_add(
                root_proc
            ,   remote_ptr
            ,   value_ptr
            ,   result_ptr
            );
            
            clocks += sw.elapsed();
        }
    }
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == root_proc) {
        const mgcom::rma::atomic_default_t val = *local_ptr;
        if (val != (number_of_trials * (mgcom::number_of_processes() - 1)))
            std::cout << "error! " << val << std::endl;
    }
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc) {
        if (proc == mgcom::current_process_id())
            std::cout << mgcom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        mgcom::collective::barrier();
    }
    
    mgcom::finalize();
    
    return 0;
}

