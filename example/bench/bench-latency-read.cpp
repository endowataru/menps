
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    mgcom::rma::local_pointer<int> local_ptr
        = mgcom::rma::allocate<int>();
    
    const mgcom::process_id_t root_proc = 0;
    mgcom::collective::broadcast(root_proc, &local_ptr, 1);
    
    mgcom::rma::remote_pointer<int> remote_ptr
        = mgcom::rma::use_remote_pointer(root_proc, local_ptr);
    
    double clocks = 0;
    
    const int number_of_trials = atoi(argv[1]);
    
    if (mgcom::current_process_id() == root_proc)
        *local_ptr = 123;
    
    mgcom::collective::barrier();
    
    mgcom::rma::local_pointer<int> local_buf
        = mgcom::rma::allocate<int>();
    
    for (int i = 0; i < number_of_trials; i++) {
        mgbase::stopwatch sw;
        sw.start();
        
        mgcom::rma::remote_read_cb cb;
        mgcom::rma::remote_read_nb(
            cb
        ,   root_proc
        ,   remote_ptr
        ,   local_buf
        )
        .wait();
        
        clocks += sw.elapsed();
        
        if (*local_buf != 123)
            std::cout << "error!" << std::endl;
    }
    
    mgcom::collective::barrier();
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc) {
        if (proc == mgcom::current_process_id())
            std::cout << mgcom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        mgcom::collective::barrier();
    }
    
    mgcom::finalize();
    
    return 0;
}

