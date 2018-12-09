
#include <menps/mecom.hpp>
#include <mpi.h>
#include <menps/mefdn/profiling/stopwatch.hpp>

int main(int argc, char* argv[])
{
    using namespace mecom;
    using namespace mecom::rma;
    using namespace mecom::rma::untyped;
    
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
                 MPI_COMM_WORLD, nullptr);
    
    remote_region remote_reg = use_remote_region(other, remote_key);
    
    //local_address  local_addr  = to_address(local_reg);
    remote_address remote_addr = to_address(remote_reg);
    
    mefdn::stopwatch sw;
    
    //uint64_t y = 1000;
    
    if (current == 0) {
        //x = 123;
        
        sw.start();
        
        registered_buffer buf   = allocate(3 * sizeof(mefdn::uint64_t));
        local_address     laddr = to_address(buf);
        mefdn::uint64_t* ptr = static_cast<mefdn::uint64_t*>(to_pointer(laddr));
        
        ptr[0] = 0;
        ptr[1] = 123;
        
        remote_compare_and_swap_default_cb cb;
        remote_compare_and_swap_default_nb(cb,
            1, remote_addr,
            laddr, advanced(laddr, sizeof(mefdn::uint64_t)),
            advanced(laddr, 2 * sizeof(mefdn::uint64_t)));
        mefdn::control::wait(cb);
        
        std::cout << mecom::current_process_id() << " " << ptr[0] << " " << ptr[1] << " " << ptr[2] << std::endl;
        
        ptr[0] = 1;
        ptr[1] = 0;
        
        remote_fetch_and_add_default_cb cb2;
        remote_fetch_and_add_default_nb(cb2, 1, remote_addr, laddr, advanced(laddr, sizeof(mefdn::uint64_t)));
        mefdn::control::wait(cb2);
        
        deallocate(buf);
        
        std::cout << current_process_id() << " " << ptr[0] << " " << ptr[1] << " " << ptr[2] << std::endl;
    }
    
    mecom::collective::barrier();
    
    std::cout << current_process_id() << " " << x << " " << sw.elapsed() << std::endl;
    
    
    finalize();
    
    return 0;
}

