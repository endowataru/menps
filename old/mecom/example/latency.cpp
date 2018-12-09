
#include <menps/mecom.hpp>
#include <mpi.h>
#include <menps/mefdn/profiling/stopwatch.hpp>

int main(int argc, char* argv[])
{
    using namespace mecom;
    using namespace mecom::rma;
    using namespace mecom::rma::untyped;
    
    initialize(&argc, &argv);
    
    int x = 0;
    local_region local_reg = register_region(&x, sizeof(x));
    
    const region_key local_key = to_region_key(local_reg);
    region_key remote_key;
    
    process_id_t current = current_process_id();
    process_id_t other = 1 - current_process_id();
    MPI_Sendrecv(&local_key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 &remote_key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 MPI_COMM_WORLD, nullptr);
    
    remote_region remote_reg = use_remote_region(other, remote_key);
    
    local_address  local_addr  = to_address(local_reg);
    remote_address remote_addr = to_address(remote_reg);
    
    mefdn::stopwatch sw;
    
    if (current == 0) {
        x = 123;
        
        sw.start();
        /*bool finished = false;
        while (!try_write_async(local_addr, remote_addr, sizeof(x), other, make_notifier_assign(&finished, true))) {
            std::cout << "write failed" << std::endl;
            poll();
        }
        
        while (!finished) {
            std::cout << "poll" << std::endl;
            poll();
        }*/
        
        remote_write_cb cb;
        remote_write_nb(cb, other, remote_addr, local_addr, sizeof(x));
        mefdn::control::wait(cb);
        
        
        
    }
    
    barrier();
    
    std::cout << x << std::endl;
    std::cout << sw.elapsed() << std::endl;
    
    
    finalize();
    
    return 0;
}

