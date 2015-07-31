
#include <mgcom.hpp>
#include <mgbase/stdint.hpp>
#include <mpi.h>

int main(int argc, char* argv[])
{
    using namespace mgcom;
    
    initialize(&argc, &argv);
    
    int x = 0;
    local_region_t local_region = register_region(&x, sizeof(x));
    
    region_key_t remote_key;
    
    process_id_t current = current_process_id();
    process_id_t other = 1 - current_process_id();
    MPI_Sendrecv(&local_region.key, sizeof(region_key_t), MPI_BYTE,
                 other, 0,
                 &remote_key, sizeof(region_key_t), MPI_BYTE,
                 other, 0,
                 MPI_COMM_WORLD, MGBASE_NULLPTR);
    
    local_address_t local_addr = { local_region, 0 };
    remote_address_t remote_addr = { use_remote_region(other, remote_key, sizeof(x)), 0 };
    
    if (current == 0) {
        x = 123;
        
        bool finished = false;
        while (!try_write_async(local_addr, remote_addr, sizeof(x), other, make_notifier(&finished, true))) {
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

