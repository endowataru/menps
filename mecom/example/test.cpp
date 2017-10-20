
#include <menps/mecom.hpp>
#include <mpi.h>

int main(int argc, char* argv[])
{
    using namespace mecom;
    
    initialize(&argc, &argv);
    
    int x = 0;
    local_region local_region = register_region(&x, sizeof(x));
    
    region_key remote_key;
    
    process_id_t current = current_process_id();
    process_id_t other = 1 - current_process_id();
    MPI_Sendrecv(&local_region.key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 &remote_key, sizeof(region_key), MPI_BYTE,
                 static_cast<int>(other), 0,
                 MPI_COMM_WORLD, nullptr);
    
    local_address  local_addr  = to_address(local_region);
    remote_address remote_addr = to_address(use_remote_region(other, remote_key));
    
    if (current == 0) {
        x = 123;
        
        bool finished = false;
        while (!try_write_async(local_addr, remote_addr, sizeof(x), other, make_notifier_assign(&finished, true))) {
            std::cout << "write failed" << std::endl;
            poll();
        }
        
        while (!finished) {
            std::cout << "poll" << std::endl;
            poll();
        }
    }
    
    barrier();
    
    std::cout << x << std::endl;
    
    finalize();
    
    return 0;
}

