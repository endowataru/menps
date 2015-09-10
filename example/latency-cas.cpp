
#include <mgcom.hpp>
#include <mpi.h>
#include <mgbase/profiling/stopwatch.hpp>

int main(int argc, char* argv[])
{
    using namespace mgcom;
    using namespace mgcom::rma;
    
    initialize(&argc, &argv);
    
    uint64_t x = 0;
    local_region local_reg = register_region(&x, sizeof(x));
    
    const region_key local_key = to_region_key(local_reg);
    region_key remote_key;
    
    process_id_t current = current_process_id();
    process_id_t other = 1 - current_process_id();
    MPI_Sendrecv(&local_key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 &remote_key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 MPI_COMM_WORLD, MGBASE_NULLPTR);
    
    remote_region remote_reg = use_remote_region(other, remote_key);
    
    //local_address  local_addr  = to_address(local_reg);
    remote_address remote_addr = to_address(remote_reg);
    
    mgbase::stopwatch sw;
    
    uint64_t y = 1000;
    
    if (current == 0) {
        //x = 123;
        
        sw.start();
        
        compare_and_swap_64_cb cb;
        compare_and_swap_64_nb(cb, remote_addr, 0, 123, &y, 1);
        mgbase::control::wait(cb);
        
        fetch_and_op_64_cb cb2;
        fetch_and_add_64_nb(cb2, remote_addr, 100, &y, 1);
        mgbase::control::wait(cb2);
        
    }
    
    barrier();
    
    std::cout << mgcom::current_process_id() << " " << x << " " << y << std::endl;
    std::cout << sw.elapsed() << std::endl;
    
    
    finalize();
    
    return 0;
}

