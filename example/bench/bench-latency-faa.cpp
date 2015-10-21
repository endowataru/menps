
#include <mgcom.hpp>
#include <mgcom_collective.hpp>
#include <mgbase/profiling/stopwatch.hpp>

int main(int argc, char* argv[])
{
    using namespace mgcom;
    using namespace mgcom::rma;
    
    initialize(&argc, &argv);
    
    atomic_default_t x = 0;
    const local_region local_reg = register_region(&x, sizeof(x));
    const local_address local_addr = to_address(local_reg);
    
    const process_id_t root_proc = 0;
    local_address root_local_addr = local_addr;
    mgcom::collective::broadcast(root_proc, &root_local_addr, 1);
    
    const remote_address root_remote_addr = use_remote_address(root_proc, root_local_addr);
    
    double clocks = 0;
    const int number_of_trials = atoi(argv[1]);
    
    registered_buffer value_buf = allocate(sizeof(atomic_default_t));
    registered_buffer result_buf = allocate(sizeof(atomic_default_t));
    *static_cast<atomic_default_t*>(to_pointer(value_buf)) = 1;
    
    barrier();
    
    if (current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mgbase::stopwatch sw;
            sw.start();
            
            remote_fetch_and_add_default_cb cb;
            remote_fetch_and_add_default_nb(cb, root_proc, root_remote_addr, to_address(value_buf), to_address(result_buf));
            mgbase::control::wait(cb);
            
            clocks += sw.elapsed();
        }
    }
    
    barrier();

    if (current_process_id() == root_proc)
        if (x != (number_of_trials * (number_of_processes() - 1)))
            std::cout << "error! " << x << std::endl;
    
    std::cout << current_process_id() << " " << (clocks / number_of_trials) << std::endl;
    
    finalize();
    
    return 0;
}

