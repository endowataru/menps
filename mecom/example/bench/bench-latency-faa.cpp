
#include <menps/mecom.hpp>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <iostream>

namespace mefdn = menps::mefdn;
namespace mecom = menps::mecom;

int main(int argc, char* argv[])
{
    mecom::initialize(&argc, &argv);
    
    mecom::rma::local_ptr<mecom::rma::atomic_default_t>
        local_ptr = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
    
    *local_ptr = 0;
    
    const mecom::process_id_t root_proc = 0;
    mecom::collective::broadcast(root_proc, &local_ptr, 1);
    
    mecom::rma::remote_ptr<mecom::rma::atomic_default_t>
        remote_ptr = mecom::rma::use_remote_ptr(root_proc, local_ptr);
    
    double clocks = 0;
    const int number_of_trials = atoi(argv[1]);
    
    mecom::rma::local_ptr<mecom::rma::atomic_default_t>
        value_ptr = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
    
    mecom::rma::local_ptr<mecom::rma::atomic_default_t>
        result_ptr = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
    
    *value_ptr = 1;
    
    mecom::collective::barrier();
    
    if (mecom::current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mefdn::stopwatch sw;
            sw.start();
            
            mecom::rma::fetch_and_add<mecom::rma::atomic_default_t>(
                root_proc
            ,   remote_ptr
            ,   *value_ptr
            ,   result_ptr
            );
            
            clocks += sw.elapsed();
        }
    }
    
    mecom::collective::barrier();
    
    if (mecom::current_process_id() == root_proc) {
        const mecom::rma::atomic_default_t val = *local_ptr;
        if (val != (number_of_trials * (mecom::number_of_processes() - 1)))
            std::cout << "error! " << val << std::endl;
    }
    
    for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc) {
        if (proc == mecom::current_process_id())
            std::cout << mecom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        mecom::collective::barrier();
    }
    
    mecom::finalize();
    
    return 0;
}

