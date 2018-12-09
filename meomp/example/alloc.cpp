
#include <menps/meomp.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <omp.h>
#include <iostream>

extern "C"
int meomp_main(int argc, char** argv)
{
    const int num = 1024;
    
    auto p = static_cast<int*>(meomp_malloc(sizeof(int)*num));
    
    #pragma omp parallel
    {
        for (int i = 0; i < 100; ++i) {
            #pragma omp for
            for (int j = 0; j < num; ++j) {
                ++p[j];
            }
            
            std::cout << omp_get_thread_num() << " " << i << std::endl;
        }
    }
    
    meomp_free(p);
}

