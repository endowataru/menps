
#include <iostream>
#include <omp.h>

#include <menps/meomp.hpp>

MEOMP_GLOBAL_VAR int g_v = 123;

extern "C"
int meomp_main(int argc, char** argv)
{
    int x = 123;
    int arr[omp_get_num_threads()];
    //int arr[5] = {};
    
    std::fill(arr, arr+omp_get_num_threads(), 0);
    
    #pragma omp parallel
    {
        std::cout << omp_get_thread_num() << "=" << x << std::endl;
        
        #if 0
        std::cout << "data[0]=" << *(std::uint64_t*)(0x100003f20) <<  std::endl;
        std::cout << "data[1]=" << *(std::uint64_t*)(0x100003f20 + 8) <<  std::endl;
        
        std::uint8_t sum = 0;
        for (std::uint8_t* ptr = (std::uint8_t*)0x100000000;
             ptr < (std::uint8_t*)0x100004000;
             ++ptr)
        {
            sum += *ptr;
        }
        std::cout << "sum=" << (int)sum << std::endl;
        
        
        std::cout << std::hex << (std::uintptr_t)arr << std::endl;
        #endif
        
        arr[omp_get_thread_num()] = omp_get_num_threads();
        
        #pragma omp barrier
        
        if (omp_get_thread_num() == 1)
            g_v = 100;
        
       //#pragma omp barrier
        
        //std::cout << omp_get_thread_num() << std::endl;
    }
    
    for (int i = 0; i < omp_get_num_threads(); ++i) {
        std::cout << arr[i] << ",";
    }
    std::cout << std::endl;
    
    std::cout << g_v << std::endl;
    
    return 0;
}


